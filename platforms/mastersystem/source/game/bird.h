#ifndef BIRD_INCLUDE_H
#define BIRD_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

// It's a bird! It's a bat! It's a flying tomato!

#define BIRD_INACTIVE			0
#define BIRD_ACTIVE				1
#define BIRD_SHUTDOWN			2

typedef struct
{
	dl_u8 state;	// 0 - inactive
					// 1 - active
	dl_u16 x; // high resolution position 256 pixels, 256 subpixels
	dl_u16 y; // high resolution position 256 pixels, 256 subpixels
	dl_u16 speedx;
	dl_u16 speedy; // high resolution
	dl_u8 animationCounter;
	dl_u8 animationFrame;
} BirdData;

void Bird_Init(BirdData* birdData);
void Bird_Update(BirdData* birdData, dl_u16 currentRoomTimer);

#endif