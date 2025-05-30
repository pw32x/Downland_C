#ifndef DRAW_UTILS_INCLUDE_H
#define DRAW_UTILS_INCLUDE_H

#include "base_types.h"
#include "background_types.h"
#include "resource_types.h"

#define CRT_EFFECT_MASK 0x55 // 01010101b

extern u8 pixelMasks[4];

// Set or clear a pixel in the 1-bit framebuffer
void setPixel(u8* framebuffer, s16 x, s16 y, u8 value);

void drawText(const u8* text, const u8* characterFont, u8* framebuffer, u16 framebufferPosition);

void drawSprite_16PixelsWide(const u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);
void drawSprite_24PixelsWide(const u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);
void drawSprite_24PixelsWide_noblend(const u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);

void drawSprite_24PixelsWide_static(const u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer);
void drawSprite_16PixelsWide_static_IntoSpriteBuffer(const u8* spriteData, u8 numLines, u8* spriteBuffer);

void eraseSprite_16PixelsWide(const u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);
void eraseSprite_16PixelsWide_simple(u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);
void eraseSprite_24PixelsWide(const u8* spriteData, u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);
void eraseSprite_24PixelsWide_simple(u8 x, u8 y, u8 numLines, u8* framebuffer, u8* cleanBackground);

const u8* getBitShiftedSprite(const u8* bitShiftedSpriteData, u8 frameNumber, u8 x, u8 spriteFrameSize);

void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const Resources* resources,
					u8* framebuffer);

#endif