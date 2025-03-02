#ifndef BIRD_INCLUDE_H
#define BIRD_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

#define BIRD_SPRITE_COUNT			2
#define BIRD_SPRITE_ROWS			6
#define BIRD_COLLISION_WIDTH		8

typedef struct
{
	u8 state;	// 0 - inactive
				// 1 - active
	u16 x; // high resolution position 256 pixels, 256 subpixels
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u16 speedx;
	u16 speedy; // high resolution
	u8* currentSprite;
	u8* bitShiftedSprites;
	u8 animationCounter;
	u8 animationFrame;
} BirdData;

void Bird_Init(BirdData* birdData, u8 roomNumber, Resources* resources);
void Bird_Update(BirdData* birdData, u16 currentRoomTimer, u8* framebuffer, u8* cleanBackground);

#endif