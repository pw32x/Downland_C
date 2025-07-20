#include "display.h"

#include "displayutils.h"
#include "mem.h"
#include "abort.h"
#include "io.h"

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