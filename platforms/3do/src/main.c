/*
  ISC License

  Copyright (c) 2024, Antonio SJ Musumeci <trapexit@spawn.link>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
 

#include "hardware.h"
#include "operamath.h"
//#include <stdio.h>
//#include <stdarg.h>
#include "BlockFile.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "types.h"
#include "filestream.h"
#include "filestreamfunctions.h"

#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"

#include "celutils.h"
#include "displayutils.h"
#include "types.h"
#include "varargs.h"
#include "graphics.h"
#include "io.h"
#include "abort.h"
#include "stdio.h"
#include "mem.h"

#include "image_utils.h"

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

static
int
clamp(const frac16  min_,
      const frac16  max_,
      frac16*       val_)
{
    if (*val_ > max_)
    {
        *val_ = max_;
        return 1;
    }

    if (*val_ < min_)
    {
        *val_ = min_;
        return 1;
    }

  return 0;
}

static
void
ZoomRotateCel(CCB    *ccb_,
              frac16  x_,
              frac16  y_,
              frac16  zoom_,
              frac16  angle_)
{
  int hdx;
  int hdy;
  int vdx;
  int vdy;

  hdx = MulSF16(CosF16(angle_),zoom_);
  hdy = MulSF16(SinF16(angle_),zoom_);
  vdx = -hdy;
  vdy =  hdx;

  ccb_->ccb_HDX = hdx << 4;
  ccb_->ccb_HDY = hdy << 4;
  ccb_->ccb_VDX = vdx;
  ccb_->ccb_VDY = vdy;

  hdx = MulSF16(hdx,Convert32_F16(ccb_->ccb_Width) >> 1);
  hdy = MulSF16(hdy,Convert32_F16(ccb_->ccb_Width) >> 1);
  vdx = MulSF16(vdx,Convert32_F16(ccb_->ccb_Height) >> 1);
  vdy = MulSF16(vdy,Convert32_F16(ccb_->ccb_Height) >> 1);

  ccb_->ccb_XPos = x_ - hdx - vdx;
  ccb_->ccb_YPos = y_ - hdy - vdy;
}


#define NUM_SCREENS 2

ScreenContext *sc;
int  _screen;
Item _vram_ioreq;
Item _vbl_ioreq;


void InitBasicDisplay()
{
    Item rv;
    Err err;
    int i = 0;

    err = OpenGraphicsFolio();
    if(err < 0)
        abort_err(err);

    sc = (ScreenContext*)AllocMem(sizeof(ScreenContext),MEMTYPE_ANY);
    if(sc == NULL)
        return;

    rv = CreateBasicDisplay(sc,DI_TYPE_DEFAULT,NUM_SCREENS);

    if(rv < 0)
    {
        FreeMem(sc,sizeof(ScreenContext));
        sc = NULL;
        abort("unable to initialize display");
    }
    
    for (i = 0; i < sc->sc_NumScreens; i++)
    {
        DisableHAVG(sc->sc_Screens[i]);
        DisableVAVG(sc->sc_Screens[i]);
    }

    _vram_ioreq = CreateVRAMIOReq();
    _vbl_ioreq  = CreateVBLIOReq();
    _screen     = 0;

}

void ShutdownBasicDisplay()
{
  if(sc == NULL)
    return;

  DeleteIOReq(_vbl_ioreq);
  DeleteIOReq(_vram_ioreq);
  DeleteBasicDisplay(sc);
  FreeMem(sc,sizeof(ScreenContext));
}

Err clear(const int color_)
{
  return SetVRAMPages(_vram_ioreq,
                      sc->sc_Bitmaps[_screen]->bm_Buffer,
                      color_,
                      75,
                      0xFFFFFFFF);
}

Err waitvbl()
{
  return WaitVBL(_vbl_ioreq,1);
}

Err display()
{
  return DisplayScreen(sc->sc_Screens[_screen],0);
}

void swap()
{
  _screen = !_screen;
}

Err display_and_swap()
{
  Err err;

  err = display();
  swap();

  return err;
}

Err draw_cels(CCB *ccb_)
{
  return DrawCels(sc->sc_BitmapItems[_screen],ccb_);
}

// Uses DrawText8 and is therefore slow
Err
draw_text8(const int   x_,
           const int   y_,
           const char *str_)
{
  Err err;
  GrafCon gcon;
  Item bitmapItem;

  gcon.gc_PenX = x_;
  gcon.gc_PenY = y_;

  bitmapItem = sc->sc_BitmapItems[_screen];

  err = DrawText8(&gcon,bitmapItem,(const u8*)str_);

  return 0;
}

Err
draw_printf(const int x_,
            const int y_,
            const char *fmt_,
            ...)
{
  va_list args;
  char strbuf[256];

  va_start(args,fmt_);
  vsprintf(strbuf,fmt_,args);
  va_end(args);

  return draw_text8(x_,y_,strbuf);
}

Err
draw_vprintf(const int   x_,
             const int   y_,
             const char *fmt_,
             va_list     args_)
{
  char strbuf[256];

  vsprintf(strbuf,fmt_,args_);

  return draw_text8(x_,y_,strbuf);
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

#define READ_SIZE 4096
int bytesRead = 0;
int someCount = 0;

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

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define CEL_WIDTH     16
#define CEL_HEIGHT    16

// Scaling factor
#define SCALE_FACTOR  4

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


    InitCel(&crtFramebufferCCB, 256, 192, 8, INITCEL_CODED);
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
    CCB *logo;

    const int clearColor = 0x00000000;
    frac16       x       ;
    frac16       y       ;
    frac16       angle   ;
    frac16       zoom    ;
    frac16       dzoom   ;
    const frac16 max_zoom = Convert32_F16(3);
    const frac16 min_zoom = (Convert32_F16(1) >> 6);
    char bitstr[33];

    dl_u8 segmentCount;
    const ShapeSegment* segments;

    bool romFoundAndLoaded = false;
    int loop;
    int drawCommandCount = 0;
    const BackgroundDrawCommand* backgroundDrawCommands;

    g_memory = (dl_u8*)malloc(DOWNLAND_MEMORY_SIZE);
    g_memoryEnd = g_memory;

    g_crtFramebuffer = (dl_u8*)malloc(FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

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

    logo = LoadCel("art/3do_logo_unpacked.cel",MEMTYPE_CEL);

    clear(clearColor);
    swap();

    x        = sc->sc_BitmapWidth >> 1;
    y        = sc->sc_BitmapHeight >> 1;
    angle    = 0;
    zoom     = 0;
    dzoom    = DivSF16(Convert32_F16(1),Convert32_F16(200));

    Game_Init(&gameData, &resources);
    InitCCBs();

    while(true)
    {
        Game_Update(&gameData, &resources);

        //clear(clearColor);
        //draw_cels(&framebufferCCB);

        convert1bppImageTo8bppCrtEffectImage(gameData.framebuffer,
                                             g_crtFramebuffer,
                                             FRAMEBUFFER_WIDTH,
                                             FRAMEBUFFER_HEIGHT,
                                             CrtColor_Blue);

        draw_cels(&crtFramebufferCCB);
        //draw_cels(logo);

        //draw_cels(&myCCB);

        //draw_printf(16,16,"x: %d",ConvertF16_32(x));
        //draw_printf(16,24,"y: %d",ConvertF16_32(y));
        //draw_printf(16,32,"x: %d",ConvertF16_32(zoom));
        //draw_printf(16,48,"y: %d",ConvertF16_32(angle));

        
        //int_to_bits(n, bitstr, 32);

        /*
        //CCB* ccb = &myCCB;
        pCCB = &myCCB;
        pCCB = logo;
        

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
        

        //drawCommandCount = resources.roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData.drawCommandCount;
        //draw_printf(16, 0, "command count %d", drawCommandCount);

        //backgroundDrawCommands = resources.roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData.backgroundDrawCommands;

        /*
        for (loop = 0; loop < drawCommandCount; loop++)
        {
            draw_printf(16, 16 + (16 * loop), 
                        "shape: %d, count %d", backgroundDrawCommands->shapeCode, backgroundDrawCommands->drawCount);
            backgroundDrawCommands++;
        }
        */

        /*
        segmentCount = resources.shapeDrawData_00_Stalactite.segmentCount;
        segments = resources.shapeDrawData_00_Stalactite.segments;
        draw_printf(16, 0, "segmentCount %d", segmentCount);
        
        for (loop = 0; loop < segmentCount; loop++)
        {
            draw_printf(16, 16 + (16 * loop), 
                        "spi: %d, pc %d, or %d", 
                        segments->subpixelIncrement,
                        segments->pixelCount,
                        segments->orientation);

            segments++;
        }
        */

	    // dl_u8 count = shapeDrawData->segmentCount;
	    // const ShapeSegment* shapeSegmentRunner = shapeDrawData->segments;

        display_and_swap();

        waitvbl();
    }

    return 0;
}
