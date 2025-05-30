#ifndef BALL_INCLUDE_H
#define BALL_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

typedef struct
{
	u8 enabled;
	u8 state;	// 0 - inactive
				// 1 - resetting?
				// 2 - active
				// 0xff - dying?
	u16 x; // high resolution position 256 pixels, 256 subpixels
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u16 speedx;
	u16 speedy; // high resolution
	const u8* currentSprite;
	u8* bitShiftedSprites;
	u8 fallStateCounter;
} BallData;

void Ball_Init(BallData* ballData, u8 roomNumber, Resources* resources);
void Ball_Update(BallData* ballData, u8* framebuffer, u8* cleanBackground);

#endif