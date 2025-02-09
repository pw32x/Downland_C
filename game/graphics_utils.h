#ifndef GRAPHICS_UTILS_INCLUDE_H
#define GRAPHICS_UTILS_INCLUDE_H

#include "base_types.h"

#define CRT_EFFECT_MASK 0x55 // 01010101b

// Set or clear a pixel in the 1-bit framebuffer
void setPixel(u8* framebuffer, u8 x, u8 y, u8 value);

void drawText(u8* text, u8* characterFont, u8* framebuffer, u16 framebufferPosition);

#endif