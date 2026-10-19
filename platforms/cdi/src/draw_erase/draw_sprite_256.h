#ifndef DRAW_SPRITE_256_INCLUDE_H
#define DRAW_SPRITE_256_INCLUDE_H

#include "base_types.h"
#include "game_sprite_type.h"

// functions to draw into a 256 pixel wide buffer
//

// converts a text grid location from the original game to the 
// corresponding pixel location in a 256 pixel wide framebuffer.
#define _1BPP_LOCATION_TO_256(xyLocation) ((xyLocation % 32) * 8) + ((xyLocation / 32) * FRAMEBUFFER_WIDTH)

void drawSprite16x16_256(dl_u16 x,
						 dl_u16 y,
						 dl_u8 frameIndex,
						 const GameSprite* gameSprite,
						 dl_u8* buffer);

void drawSprite16x10_256(dl_u16 x,
						 dl_u16 y,
						 dl_u8 frameIndex,
						 const GameSprite* gameSprite,
						 dl_u8* buffer);

void drawText8_256(dl_u32 xyLocation,
				   dl_u8 characterIndex,
				   const GameSprite* characterFont,
				   dl_u8* buffer);

void drawUIText_256(const dl_u8* text, 
					dl_u32 xyLocation, 
					const GameSprite* characterFont, 
					dl_u8* buffer);

#endif
