#include "framebuffer_utils.h"

#include "base_defines.h"

void set_pixel(u8* framebuffer, u8 x, u8 y, u8 value) 
{
    if (x < 0 || x >= FRAMEBUFFER_WIDTH || y < 0 || y >= FRAMEBUFFER_HEIGHT) 
        return;

    u8 pixel = 1 << (7 - (x % 8));
    int index = (x / 8) + (y * FRAMEBUFFER_PITCH);

    if (value)
        framebuffer[index] |= pixel;  // set bit/pixel
    else
        framebuffer[index] &= ~pixel; // clear bit/pixel
}