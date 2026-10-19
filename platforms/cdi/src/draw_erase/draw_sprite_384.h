#ifndef DRAW_SPRITE_384_INCLUDE_H
#define DRAW_SPRITE_384_INCLUDE_H

#include "base_types.h"
#include "game_sprite_type.h"

// Functions to draw into the current drawVideoBuffer
//

// converts a text grid location from the original game to the 
// corresponding pixel location in a 384 pixel wide framebuffer.
#define _1BPP_LOCATION_TO_384(xyLocation)  (((xyLocation % 32) * 8) + SCREEN_OFFSET_X) + (((xyLocation / 32) + SCREEN_OFFSET_Y) * SCREEN_WIDTH)

void drawSprite16_384(dl_u16 x,
					  dl_u16 y,
					  dl_u8 frameIndex,
					  const GameSprite* gameSprite,
					  dl_u8 addToErase);

void drawSprite16Opaque_384(dl_u16 x,
							dl_u16 y,
							dl_u8 frameIndex,
							const GameSprite* gameSprite);

void drawSpriteDrop_384(dl_u16 x,
						dl_u16 y,
						dl_u8 frameIndex,
						const GameSprite* gameSprite);

void drawSprite24_384(dl_u16 x,
					  dl_u16 y,
					  dl_u8 frameIndex,
					  const GameSprite* gameSprite);

void drawSprite8_384(dl_u16 x,
					 dl_u16 y,
					 dl_u8 frameIndex,
					 const GameSprite* gameSprite);

void drawText8_384(dl_u32 xyLocation,
				   dl_u8 characterIndex,
				   const GameSprite* characterFont);

void drawUIText_384(const dl_u8* text, 
				    dl_u32 xyLocation, 
				    const GameSprite* characterFont);

#endif
