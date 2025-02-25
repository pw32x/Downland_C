#ifndef DEBUG_UTILS_INCLUDE_H
#define DEBUG_UTILS_INCLUDE_H

#include "base_defines.h"

#ifdef _WINDOWS

unsigned int debugFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
void debugSetPixel(unsigned char x, unsigned char y, unsigned int pixel);
void debugDrawBox(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned int pixel);

void DebugPrintf(const char *format, ...);
#endif

#endif