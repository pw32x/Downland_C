#ifndef BUILD_RESOURCE_UTILS_INCLUDE_H
#define BUILD_RESOURCE_UTILS_INCLUDE_H

#include "draw_sprite_utils.h"
#include "resource_types.h"

void buildSpriteResource(GameSprite* gameSprite,
						 const dl_u8* originalSprite,
						 dl_u8 frameWidth,
						 dl_u8 frameHeight,
						 dl_u8 spriteCount);

void buildTextResource(GameSprite* gameSprite,
					   const dl_u8* originalSprite,
					   dl_u8 frameWidth,
					   dl_u8 frameHeight,
					   dl_u8 spriteCount);

void buildPlayerIconResource(GameSprite* gameSprite,
							 const dl_u8* originalSprite,
							 dl_u8 frameWidth,
							 dl_u8 frameHeight,
							 dl_u8 clipHeight,
							 dl_u8 spriteCount);

void buildRegenSprites(GameSprite* gameSprite,
					   const dl_u8* originalSprite,
					   dl_u8 frameWidth,
					   dl_u8 frameHeight,
					   dl_u8 spriteCount);

void buildSplatSpriteResource(const Resources* resources);

#endif
