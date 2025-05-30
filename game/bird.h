#ifndef BIRD_INCLUDE_H
#define BIRD_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

// It's a bird! It's a bat! It's a flying tomato!

typedef struct
{
	u8 state;	// 0 - inactive
				// 1 - active
	u16 x; // high resolution position 256 pixels, 256 subpixels
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u16 speedx;
	u16 speedy; // high resolution
	const u8* currentSprite;
	const u8* bitShiftedSprites;
	u8 animationCounter;
	u8 animationFrame;
} BirdData;

void Bird_Init(BirdData* birdData, u8 roomNumber, const Resources* resources);
void Bird_Update(BirdData* birdData, u16 currentRoomTimer, u8* framebuffer, u8* cleanBackground);

#endif