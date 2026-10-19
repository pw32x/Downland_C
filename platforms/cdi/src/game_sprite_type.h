#ifndef GAME_SPRITE_TYPE_INCLUDE_H
#define GAME_SPRITE_TYPE_INCLUDE_H

#include "base_types.h"

typedef struct
{
	dl_u8* spriteData;
	dl_u8 frameWidth;
	dl_u8 frameHeight;
	dl_u16 sizePerFrame;
} GameSprite;

#endif
