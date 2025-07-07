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

#include "game_types.h"
#include "resource_loader_buffer.h"

GameData gameData;
Resources resources;

#include "celutils.h"
#include "displayutils.h"
#include "types.h"
#include "varargs.h"
#include "graphics.h"
#include "io.h"
#include "abort.h"
#include "stdio.h"
#include "mem.h"


static dl_u8 memory[18288];
static dl_u8* memoryEnd = NULL;

void* dl_alloc(dl_u32 size)
{
    dl_u8* memory;

	if (memoryEnd == NULL)
	{
		memoryEnd = memory;
	}

	memory = memoryEnd;

	memoryEnd += size;

	return (void*)memory;
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


int main(int argc, char* argv)
{
    CCB *logo;
    const int clearColor = 0x00000000;
    frac16       x       ;
    frac16       y       ;
    frac16       angle   ;
    frac16       zoom    ;
    frac16       dzoom   ;
    const frac16 max_zoom = Convert32_F16(3);
    const frac16 min_zoom = (Convert32_F16(1) >> 6);

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

    while(true)
    {
        // do work
        ZoomRotateCel(logo, x, y,zoom,angle);

        angle += Convert32_F16(1);
        zoom  += dzoom;
        if (clamp(min_zoom, max_zoom, &zoom))
        {
            dzoom = -dzoom;
            x = Convert32_F16(ReadHardwareRandomNumber() % sc->sc_BitmapWidth);
            y = Convert32_F16(ReadHardwareRandomNumber() % sc->sc_BitmapHeight);
            kprintf("x,y: %d, %d\n",
                    ConvertF16_32(x),
                    ConvertF16_32(y));
        }

        Game_Update(&gameData, &resources);

        // draw

        clear(clearColor);
        draw_cels(logo);
        draw_printf(16,16,"x: %d",ConvertF16_32(x));
        draw_printf(16,24,"y: %d",ConvertF16_32(y));
        display_and_swap();

        waitvbl();
    }

    return 0;
}
