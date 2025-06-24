#ifndef BIRD_INCLUDE_H
#define BIRD_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

// It's a bird! It's a bat! It's a flying tomato!

typedef struct
{
	dl_u8 state;	// 0 - inactive
				// 1 - active
	dl_u16 x; // high resolution position 256 pixels, 256 subpixels
	dl_u16 y; // high resolution position 256 pixels, 256 subpixels
	dl_u16 speedx;
	dl_u16 speedy; // high resolution
	const dl_u8* currentSprite;
	const dl_u8* bitShiftedSprites;
	dl_u8 animationCounter;
	dl_u8 animationFrame;
} BirdData;

void Bird_Init(BirdData* birdData, dl_u8 roomNumber, const Resources* resources);
void Bird_Update(BirdData* birdData, dl_u16 currentRoomTimer, dl_u8* framebuffer, dl_u8* cleanBackground);

#endif