#include "debug_utils.h"

const unsigned char placeHolder = 3; // SDCC warning 190: ISO C forbids an empty translation unit

#ifdef DEV_MODE
#ifdef _WINDOWS

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

extern unsigned int debugFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

void debugSetPixel(unsigned char x, unsigned char y, unsigned int pixel)
{
    debugFramebuffer[x + (y * FRAMEBUFFER_WIDTH)] = pixel;
}

void debugDrawBox(unsigned char x, 
                  unsigned char y, 
                  unsigned char width, 
                  unsigned char height, 
                  unsigned int pixel)
{
    for (int loop = 0; loop < width; loop++)
    {
        debugFramebuffer[x + loop + (y * FRAMEBUFFER_WIDTH)] = pixel;
        debugFramebuffer[x + loop + ((y + height) * FRAMEBUFFER_WIDTH)] = pixel;
    }

    for (int loop = 0; loop < height; loop++)
    {
        debugFramebuffer[x + ((y + loop) * FRAMEBUFFER_WIDTH)] = pixel;
        debugFramebuffer[x + width + ((y + loop) * FRAMEBUFFER_WIDTH)] = pixel;
    }
}

void DebugPrintf(const char *format, ...) 
{
    char buffer[512];  // Buffer for formatted string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    OutputDebugStringA(buffer);
}
#endif
#endif
