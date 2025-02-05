#ifndef FRAMEBUFFER_UTILS_INCLUDE_H
#define FRAMEBUFFER_UTILS_INCLUDE_H

#include "base_types.h"

// Set or clear a pixel in the 1-bit framebuffer
void set_pixel(u8* framebuffer, u8 x, u8 y, u8 value);

#endif