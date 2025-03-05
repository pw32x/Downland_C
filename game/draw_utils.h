#ifndef DRAW_UTILS_INCLUDE_H
#define DRAW_UTILS_INCLUDE_H

#include "base_types.h"
#include "background_types.h"
#include "resource_types.h"

#define CRT_EFFECT_MASK 0x55 // 01010101b

extern u8 pixelMasks[4];

// Set or clear a pixel in the 1-bit framebuffer
void setPixel(u8* framebuffer, u8 x, u8 y, u8 value);

void drawText(u8* text, u8* characterFont, u8* framebuffer, u16 framebufferPosition);

void drawSprite_16PixelsWide(u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);
void drawSprite_24PixelsWide(u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);
void drawSprite_24PixelsWide_noblend(u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);
void drawSprite_24PixelsWide_static(u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);

void eraseSprite_16PixelsWide(u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);
void eraseSprite_16PixelsWide_simple(u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);
void eraseSprite_24PixelsWide(u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);
void eraseSprite_24PixelsWide_simple(u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);

u8* getBitShiftedSprite(u8* bitShiftedSpriteData, u8 frameNumber, u8 x, u8 spriteFrameSize);

void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const Resources* resources,
					u8* framebuffer);

#endif