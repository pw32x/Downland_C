#ifndef DRAW_UTILS_INCLUDE_H
#define DRAW_UTILS_INCLUDE_H

#include "base_types.h"
#include "background_types.h"
#include "resource_types.h"

extern const dl_u8 pixelMasks[4];

#ifdef ENABLE_CLEAN_BACKGROUND_DRAW
void drawText(const dl_u8* text, const dl_u8* characterFont, dl_u8* framebuffer, dl_u16 framebufferPosition);

void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const BackgroundShapeDrawData* backgroundshapedrawdata,
					dl_u8* backgroundBuffer);
#endif

#endif