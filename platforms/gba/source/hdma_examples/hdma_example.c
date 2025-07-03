#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Sine table for smooth wave effect (256 entries for full circle)
const s16 sine_table[256] = 
{
    0, 3, 6, 9, 12, 15, 18, 21, 24, 28, 31, 34, 37, 40, 43, 46,
    48, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88,
    90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 109, 111, 112, 114, 115, 117,
    118, 119, 120, 121, 122, 123, 124, 124, 125, 126, 126, 127, 127, 127, 128, 128,
    128, 128, 128, 127, 127, 127, 126, 126, 125, 124, 124, 123, 122, 121, 120, 119,
    118, 117, 115, 114, 112, 111, 109, 108, 106, 104, 102, 100, 98, 96, 94, 92,
    90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51,
    48, 46, 43, 40, 37, 34, 31, 28, 24, 21, 18, 15, 12, 9, 6, 3,
    0, -3, -6, -9, -12, -15, -18, -21, -24, -28, -31, -34, -37, -40, -43, -46,
    -48, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
    -90, -92, -94, -96, -98, -100, -102, -104, -106, -108, -109, -111, -112, -114, -115, -117,
    -118, -119, -120, -121, -122, -123, -124, -124, -125, -126, -126, -127, -127, -127, -128, -128,
    -128, -128, -128, -127, -127, -127, -126, -126, -125, -124, -124, -123, -122, -121, -120, -119,
    -118, -117, -115, -114, -112, -111, -109, -108, -106, -104, -102, -100, -98, -96, -94, -92,
    -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
    -48, -46, -43, -40, -37, -34, -31, -28, -24, -21, -18, -15, -12, -9, -6, -3
};

// HDMA table - needs to be in IWRAM for DMA access
IWRAM_DATA u16 hdma_table[160];

// Animation variables
int wave_offset = 0;
int amplitude = 16;  // Wave amplitude
int frequency = 2;   // Wave frequency multiplier

void init_background() 
{
    // Enable BG0 in mode 0
    SetMode(MODE_0 | BG0_ON);
    
    // Set up BG0 - 32x32 tiles, 4bpp
    REG_BG0CNT = BG_SIZE_0 | BG_16_COLOR | CHAR_BASE(0) | SCREEN_BASE(31);
    
    // Load a simple palette (just a few colors for testing)
    BG_PALETTE[0] = RGB5(0, 0, 0);      // Black
    BG_PALETTE[1] = RGB5(31, 31, 31);   // White
    BG_PALETTE[2] = RGB5(31, 0, 0);     // Red
    BG_PALETTE[3] = RGB5(0, 31, 0);     // Green
    BG_PALETTE[4] = RGB5(0, 0, 31);     // Blue
    
    // Create a simple tile pattern (4bpp = 32 bytes per 8x8 tile)
    u32* tile_mem = (u32*)TILE_BASE_ADR(0);
    
    // Tile 0
    tile_mem[0] = 0x11111111;
    tile_mem[1] = 0x11111111;
    tile_mem[2] = 0x11111111;
    tile_mem[3] = 0x11111111;
    tile_mem[4] = 0x11111111;
    tile_mem[5] = 0x11111111;
    tile_mem[6] = 0x11111111;
    tile_mem[7] = 0x11111111;

    // Tile 1
    tile_mem[8] = 0x33333333; 
    tile_mem[9] = 0x33333333; 
    tile_mem[10] = 0x33333333;
    tile_mem[11] = 0x33333333;
    tile_mem[12] = 0x33333333;
    tile_mem[13] = 0x33333333;
    tile_mem[14] = 0x33333333;
    tile_mem[15] = 0x33333333;
    
    // Fill the background map with alternating tiles
    u16* bg_map = (u16*)MAP_BASE_ADR(31);

    for(int y = 0; y < 32; y++) 
    {
        for(int x = 0; x < 32; x++) 
        {
            bg_map[y * 32 + x] = (x + y) & 1;  // Checkerboard pattern of tiles
        }
    }
}

void update_hdma_table() 
{
    // Generate sine wave offsets for each scanline
    for(int y = 0; y < 160; y++) 
    {
        // Calculate sine wave offset for this scanline
        int angle = (y * frequency + wave_offset) & 0xFF;
        int offset = (sine_table[angle] * amplitude) >> 7;  // Scale down the sine value
        
        // Store the offset value (no need to mask since BG registers handle wrapping)
        hdma_table[y] = offset;
    }
}


void setup_hdma() 
{
    // Important: Reset DMA3 first
    REG_DMA3CNT = 0;
    
    // Set up HDMA on DMA channel 3
    REG_DMA3SAD = (u32)hdma_table;       // Source address
    REG_DMA3DAD = (u32)&REG_BG0HOFS;     // Destination: BG0 horizontal offset
    
    // Configure DMA3 for HDMA
    // Don't use DMA_SPECIAL. DMA_HBLANK is sufficient.
    // DMA_REPEAT repeats transfer for the whole frame. Will need to be restarted on vblank.
    // DMA_DST_FIXED keeps writing to the same register
    // DMA_SRC_INC moves through our table
    // DMA16 for 16-bit transfers
    // Transfer count of 1 for the number of words 
    // we transfer per call to the destination address.
    // REG_BG0HOFS is a 16 bit value set for every hblank
    // from this table.
    REG_DMA3CNT = DMA_ENABLE | DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA16 | 1;
}

void vblank_handler() 
{
    // Update wave animation
    wave_offset++;
    if(wave_offset >= 256) wave_offset = 0;

    // on Vblank, update the table and restart the HDMA
    //

    // Update the HDMA table with new sine wave values
    update_hdma_table();
    
    // Set up HDMA
    setup_hdma();
}

int main() 
{
    // Initialize the background
    init_background();
    
    // Set up interrupts
    // for HDMA, don't need to irqEnable(IRQ_HBLANK)
    // or REG_DISPSTAT |= LCDC_HBL; 
    // that's handled automatically
    irqInit();
    irqSet(IRQ_VBLANK, vblank_handler);
    irqEnable(IRQ_VBLANK);

    // Initialize HDMA table
    update_hdma_table();
    
    // Set up HDMA
    setup_hdma();
    
    // Main loop
    while(1) 
    {
        VBlankIntrWait();
        
        // You can adjust wave parameters here
        scanKeys();
        u16 keys = keysHeld();
        
        if(keys & KEY_UP && amplitude < 32) amplitude++;
        if(keys & KEY_DOWN && amplitude > 1) amplitude--;
        if(keys & KEY_RIGHT && frequency < 8) frequency++;
        if(keys & KEY_LEFT && frequency > 1) frequency--;
    }
    
    return 0;
}