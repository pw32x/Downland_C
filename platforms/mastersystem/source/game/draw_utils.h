#ifndef DRAW_UTILS_INCLUDE_H
#define DRAW_UTILS_INCLUDE_H

#include "base_types.h"
#include "background_types.h"
#include "resource_types.h"

extern dl_u8 pixelMasks[4];

#ifndef DISABLE_FRAMEBUFFER

#define CRT_EFFECT_MASK 0x55 // 01010101b

void drawText(const dl_u8* text, const dl_u8* characterFont, dl_u8* framebuffer, dl_u16 framebufferPosition);

void drawSprite_16PixelsWide(const dl_u8* spriteData, dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer);
void drawSprite_24PixelsWide(const dl_u8* spriteData, dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer);
void drawSprite_24PixelsWide_noblend(const dl_u8* spriteData, dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer);

void drawSprite_24PixelsWide_static(const dl_u8* spriteData, dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer);
void drawSprite_16PixelsWide_static_IntoSpriteBuffer(const dl_u8* sourceSprite, dl_u8 numLines, dl_u8* destinationSprite);

void eraseSprite_16PixelsWide(const dl_u8* spriteData, dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer, dl_u8* cleanBackground);
void eraseSprite_16PixelsWide_simple(dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer, dl_u8* cleanBackground);
void eraseSprite_24PixelsWide(const dl_u8* spriteData, dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer, dl_u8* cleanBackground);
void eraseSprite_24PixelsWide_simple(dl_u8 x, dl_u8 y, dl_u8 numLines, dl_u8* framebuffer, dl_u8* cleanBackground);

const dl_u8* getBitShiftedSprite(const dl_u8* bitShiftedSpriteData, dl_u8 frameNumber, dl_u8 x, dl_u8 spriteFrameSize);

void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const Resources* resources,
					dl_u8* framebuffer);
#endif

#endif