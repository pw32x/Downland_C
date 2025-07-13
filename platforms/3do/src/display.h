#ifndef DISPLAY_INCLUDE_H
#define DISPLAY_INCLUDE_H

#include "graphics.h"
#include "varargs.h"

void InitBasicDisplay();
void ShutdownBasicDisplay();
Err clear(const int color_);
Err waitvbl();
Err display();
void swap();
Err display_and_swap();
Err draw_cels(CCB *ccb_);

// Uses DrawText8 and is therefore slow
Err draw_text8(const int   x_, const int   y_, const char *str_);
Err draw_printf(const int x_, const int y_, const char *fmt_, ...);
Err draw_vprintf(const int   x_, const int   y_, const char *fmt_, va_list args_);


#endif

