#include "vdp.h"
#include "data/font.h"

// MD VDP ports
volatile uint16_t* const vdp_data_port = (uint16_t*) 0xC00000;
volatile uint32_t* const vdp_data_wide = (uint32_t*) 0xC00000;
volatile uint16_t* const vdp_ctrl_port = (uint16_t*) 0xC00004;
volatile uint32_t* const vdp_ctrl_wide = (uint32_t*) 0xC00004;

uint32_t fg_color = 0x11111111; /* default to color 1 for fg color */
uint32_t bg_color = 0x00000000; /* default to color 0 for bg color */

void vpd_load_palette(const uint16_t* palette, uint16_t paletteIndex)
{
    vdp_set_autoinc(2);

    *vdp_ctrl_wide = GFX_WRITE_CRAM_ADDR((uint32_t)paletteIndex * (16 * 2)); // 16 colors, 2 bytes per color

    uint32_t* paletteWide = (uint32_t*)palette;

    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;
    *vdp_data_wide = *paletteWide++;

}

uint32_t vdp_load_tileset(const uint8_t* tileset, uint16_t tilesetSize, uint32_t vdpAddress)
{
    vdp_set_autoinc(2);
    *vdp_ctrl_wide = GFX_WRITE_VRAM_ADDR((uint32_t) vdpAddress);

    uint32_t* tilesetWide = (uint32_t*)tileset;

    uint16_t tilesetSizeWide = tilesetSize >> 2;

    for (int loop = 0; loop < tilesetSizeWide; loop++)
    {
        *vdp_data_wide = *tilesetWide;
        tilesetWide++;
    }

    return vdpAddress;
}

void vdp_draw_tileset(const uint16_t* tilemap, 
                      int16_t x, 
                      int16_t y, 
                      uint16_t width, 
                      uint16_t height, 
                      uint16_t tilesetStart,
                      uint16_t paletteIndex,
                      uint16_t planeAddr)
{
    // no clipping
    if (x < 0)
        return;
    if (y < 0)
        return;
    if (x + width > PLANE_WIDTH - 1)
        return;
    if (y + height> PLANE_HEIGHT - 1)
        return;

    vdp_set_autoinc(2);

    for (int loopy = 0; loopy < height; loopy++)
    {
        uint16_t addr = planeAddr + (((x & 63) + (((y + loopy) & 31) << 6)) * 2);
        *vdp_ctrl_wide = GFX_WRITE_VRAM_ADDR((uint32_t) addr);

        for (int loopx = 0; loopx < width; loopx++)
        {
            *vdp_data_port = (*tilemap + (tilesetStart >> 5)) | (paletteIndex << 13);
            tilemap++;
        }
    }
}

void vdp_load_font()
{
    uint16_t fontTilesetSize = sizeof(font);
    uint32_t vdpAddress = 0; // start of vdp tile area

    vdp_set_autoinc(2);
    *vdp_ctrl_wide = GFX_WRITE_VRAM_ADDR((uint32_t) vdpAddress);

    uint32_t* fontTilesetWide = (uint32_t*)font;

    fontTilesetSize >>= 2;

    for (int loop = 0; loop < fontTilesetSize; loop++)
    {
        uint32_t value = *fontTilesetWide;
        uint32_t invValue = ~value;

        value &= fg_color;
        invValue &= bg_color;

        *vdp_data_wide = value | invValue;
        fontTilesetWide++;
    }
}

void put_tile_xy(uint16_t tile, uint16_t x, uint16_t y)
{
    vdp_set_autoinc(2);

    uint16_t addr = PLANE_A_ADDR + (((x & 63) + ((y & 31) << 6)) * 2);

    *vdp_ctrl_wide = GFX_WRITE_VRAM_ADDR((uint32_t) addr);
    *vdp_data_port = tile - 32;
}

void vdp_wait_vsync()
{
    while (*vdp_ctrl_port & VDP_VBLANK_FLAG);
    while (!(*vdp_ctrl_port & VDP_VBLANK_FLAG));
}

void vdp_set_horizontal_scroll(uint16_t planeAddr, int16_t scrollAmount)
{
    uint16_t vdpDestinationAddress = HSCROLL_ADDR;
    if (planeAddr == PLANE_B_ADDR) 
        vdpDestinationAddress += 2;

    *vdp_ctrl_wide = GFX_WRITE_VRAM_ADDR(vdpDestinationAddress);
    *vdp_data_port = scrollAmount;
}

void vdp_set_vertical_scroll(uint16_t planeAddr, int16_t scrollAmount)
{
    uint16_t destinationAddress = (planeAddr == PLANE_B_ADDR) ? 2 : 0;

    *vdp_ctrl_wide = GFX_WRITE_VSRAM_ADDR((uint32_t) destinationAddress);
    *vdp_data_port = scrollAmount;
}


hardware_sprite hardware_sprites[HARDWARE_SPRITE_LIMIT];
int hardware_sprite_count = 0;
hardware_sprite* current_hardware_sprite = hardware_sprites;

void vdp_init_hardware_sprites()
{
    hardware_sprite_count = 0;
    current_hardware_sprite = hardware_sprites;
}

void vdp_push_hardware_sprite(int16_t x, int16_t y, uint8_t size, uint16_t attributes)
{
    if (hardware_sprite_count >= HARDWARE_SPRITE_LIMIT)
        return;

    hardware_sprite_count++;

    current_hardware_sprite->x = x + 128;
    current_hardware_sprite->y = y + 128;
    current_hardware_sprite->size = size;
    current_hardware_sprite->link = hardware_sprite_count;
    current_hardware_sprite->attributes = attributes;

    current_hardware_sprite++;
}

void vdp_upload_hardware_sprites()
{
    hardware_sprites[hardware_sprite_count - 1].link = 0;

    // this is the worst VRAM memory copy function you've ever seen. Use DMA instead.

    vdp_set_autoinc(2);
    *vdp_ctrl_wide = GFX_WRITE_VRAM_ADDR(SPRITE_ATTR_ADDR);

    uint32_t* source = (uint32_t*)hardware_sprites;
    uint16_t copySize = (sizeof(hardware_sprite) * hardware_sprite_count) >> 2;

    for (int loop = 0; loop < copySize; loop++)
    {
        *vdp_data_wide = *source;
        source++;
    }

    vdp_init_hardware_sprites();
}
