// game headers
#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"

// 3do headers
#include "celutils.h"

// project headers
#include "image_utils.h"
#include "display.h"
#include "game_runner.h"

GameData gameData;
Resources resources;

static dl_u8* g_crtFramebuffer = NULL;

#define DOWNLAND_MEMORY_SIZE 18288
static dl_u8* g_memory = NULL;
static dl_u8* g_memoryEnd = NULL;

void* dl_alloc(dl_u32 size)
{
	dl_u8* newMemoryAlloc = g_memoryEnd;

	g_memoryEnd += size;

	return (void*)newMemoryAlloc;
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}


#define DOWNLAND_ROM_FILE_SIZE 8192
dl_u8* g_downlandRomFileBuffer = NULL;

const char* romFileNames[] = 
{
    "downland.rom",
    "downland.bin",
    "Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};
#define ROM_FILENAMES_COUNT 3

static dl_u8 loadRom(const char* romPath, dl_u8** fileBuffer)
{
    int32 fileSize;

    *fileBuffer = (dl_u8*)LoadFile(romPath, &fileSize, MEMTYPE_ANY);

    if (*fileBuffer == NULL)
        return 0;

    if (fileSize != DOWNLAND_ROM_FILE_SIZE)
        return 0;

	return TRUE;
}

uint16 twoColorPLUT[2] = 
{
    0x0000, // index 0: black (R=0, G=0, B=0)
    0x003E  // index 1: white (R=31, G=31, B=31)
};

uint16 fourColorPLUT[4] = 
{
    0x0000, // black (R=0, G=0, B=0)
    0x003E, // blue (R=0, G=0, B=31)
    0xFC80, // orange (R=31, G=20, B=0)
    0x7FFF  // white (R=31, G=31, B=31)
};

// The 3do hardware expects at least 8 bytes per row
// no matter the bit depth used. So for this 16x16 1bpp
// sprite where a row would be 2 bytes, six more bytes are
// needed to pad the row. 
// It doesn't matter what the padding bytes look like.
uint16 mySpriteBits[] = 
{
//  sprite, filler, filler, filler
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF, 
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0x8001, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF  
};

CCB myCCB;
CCB framebufferCCB;
CCB crtFramebufferCCB;

void InitCCBs(void)
{
    // Clear the CCB (important)
    memset(&myCCB, 0, sizeof(CCB));

    InitCel(&myCCB, 16, 16, 1, INITCEL_CODED);
    myCCB.ccb_SourcePtr   = (CelData *)mySpriteBits;
    myCCB.ccb_PLUTPtr = twoColorPLUT;

    
    InitCel(&framebufferCCB, 256, 192, 1, INITCEL_CODED);
    framebufferCCB.ccb_SourcePtr   = (CelData *)gameData.framebuffer;
    framebufferCCB.ccb_PLUTPtr = twoColorPLUT;


    InitCel(&crtFramebufferCCB, 256, 192, 2, INITCEL_CODED);
    //ClearFlag(crtFramebufferCCB.ccb_Flags, CCB_NOBLK);

    crtFramebufferCCB.ccb_Flags |= CCB_BGND;

    crtFramebufferCCB.ccb_SourcePtr   = (CelData *)g_crtFramebuffer;
    crtFramebufferCCB.ccb_PLUTPtr = fourColorPLUT;
}

void int_to_bits(int n, char *out, int bits)
{
    int i;
    for (i = bits - 1; i >= 0; i--) 
    {
        out[bits - 1 - i] = (n & (1 << i)) ? '1' : '0';
    }
    out[bits] = '\0';
}

int main(int argc, char* argv)
{
    CCB* pCCB;

    const int clearColor = 0x00000000;
    dl_u8 segmentCount;
    const ShapeSegment* segments;

    bool romFoundAndLoaded = false;
    int loop;

    g_memory = (dl_u8*)malloc(DOWNLAND_MEMORY_SIZE);
    g_memoryEnd = g_memory;

    g_crtFramebuffer = (dl_u8*)malloc(FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT / 4);

    for (loop = 0; loop < ROM_FILENAMES_COUNT; loop++)
    {
        if (loadRom(romFileNames[loop], &g_downlandRomFileBuffer) &&
            checksumCheckBigEndian(g_downlandRomFileBuffer, DOWNLAND_ROM_FILE_SIZE) &&
            ResourceLoaderBuffer_Init(g_downlandRomFileBuffer, DOWNLAND_ROM_FILE_SIZE, &resources))
        {
            romFoundAndLoaded = true;
            break;
        }

        break;
    }

    if (!romFoundAndLoaded)
        return -1;

    InitBasicDisplay();

    OpenMathFolio();

    clear(clearColor);
    swap();

    GameRunner_Init(&gameData, &resources);
    InitCCBs();

    while(true)
    {
        GameRunner_Update(&gameData, &resources);
        GameRunner_Draw(&gameData, &resources);

        //clear(clearColor);
        //draw_cels(&framebufferCCB);

        convert1bppImageTo2bppCrtEffectImage(gameData.framebuffer,
                                             g_crtFramebuffer,
                                             FRAMEBUFFER_WIDTH,
                                             FRAMEBUFFER_HEIGHT,
                                             CrtColor_Blue);

        draw_cels(&crtFramebufferCCB);

        //draw_cels(&myCCB);

        //draw_printf(16,16,"x: %d",ConvertF16_32(x));
        //draw_printf(16,24,"y: %d",ConvertF16_32(y));
        //draw_printf(16,32,"x: %d",ConvertF16_32(zoom));
        //draw_printf(16,48,"y: %d",ConvertF16_32(angle));

        
        //int_to_bits(n, bitstr, 32);

        /*
        //CCB* ccb = &myCCB;
        pCCB = &myCCB;

        int_to_bits(pCCB->ccb_Flags, bitstr, 32); draw_printf(0, 0, "Flags: %s", bitstr);
        int_to_bits(pCCB->ccb_XPos, bitstr, 32); draw_printf(0, 16, "XPos: %s", bitstr);
        int_to_bits(pCCB->ccb_YPos, bitstr, 32); draw_printf(0, 32, "YPos: %s", bitstr);
        int_to_bits(pCCB->ccb_HDX, bitstr, 32); draw_printf(0, 48, "HDX: %s", bitstr);
        int_to_bits(pCCB->ccb_HDY, bitstr, 32); draw_printf(0, 64, "HDY: %s", bitstr);
        int_to_bits(pCCB->ccb_VDX, bitstr, 32); draw_printf(0, 80, "VDX: %s", bitstr);
        int_to_bits(pCCB->ccb_VDY, bitstr, 32); draw_printf(0, 96, "VDY: %s", bitstr);
        int_to_bits(pCCB->ccb_HDDX, bitstr, 32); draw_printf(0, 112, "HDDX: %s", bitstr);
        int_to_bits(pCCB->ccb_HDDY, bitstr, 32); draw_printf(0, 128, "HDDY: %s", bitstr);
        int_to_bits(pCCB->ccb_PIXC, bitstr, 32); draw_printf(0, 144, "PIXC: %s", bitstr);
        int_to_bits(pCCB->ccb_PRE0, bitstr, 32); draw_printf(0, 156, "PRE0: %s", bitstr);
        int_to_bits(pCCB->ccb_PRE1, bitstr, 32); draw_printf(0, 168, "PRE1: %s", bitstr);
        int_to_bits(pCCB->ccb_Width, bitstr, 32); draw_printf(0, 180, "Width: %s", bitstr);
        int_to_bits(pCCB->ccb_Height, bitstr, 32); draw_printf(0, 192, "Height: %s", bitstr);
        */

        //draw_printf(16, 16,"bytesRead %d",bytesRead);
        //draw_printf(16, 32,"load %d",loadResult);
        //draw_printf(16, 48,"little %d",littleResult);
        //draw_printf(16, 64,"big %d",bigResult);
        
        display_and_swap();

        waitvbl();
    }

    return 0;
}
