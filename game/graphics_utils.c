#include "graphics_utils.h"

#include "base_defines.h"

void setPixel(u8* framebuffer, u8 x, u8 y, u8 value) 
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

void drawText(u8* text, u8* characterFont, u8* framebuffer, u16 framebufferPosition)
{
    u8 rowsPerCharacter = 7;
    framebuffer += framebufferPosition;

    // for each character
    while (*text != 0xff)
    {
        // find the corresponding character in the font
        u8* character = &characterFont[*text * rowsPerCharacter]; // index of the character * 7 bytes per character if font

        for (int loop = 0; loop < rowsPerCharacter; loop++)
        {
            *framebuffer = character[loop] & CRT_EFFECT_MASK;
            framebuffer += 0x20; // go down one row in the frame buffer for the next line.
        }

        framebuffer -= 0xdf;

        text++;
    }
}

void drawSprite_16PixelsWide(u8* sprite, u8 x, u8 y, u8* framebuffer)
{
    u8 rowsPerSprite = 10;

    framebuffer += (x / 4) + (y * FRAMEBUFFER_PITCH);

    // for each character
    while (rowsPerSprite--)
    {
        // first byte
        *framebuffer |= *sprite;
        framebuffer++;
        sprite++;

        // second byte
        *framebuffer |= *sprite;
        sprite++;

        // move framebuffer to next row
        framebuffer += 0x1f; // go down one row in the frame buffer for the next line.
    }
}