#ifndef BALL_INCLUDE_H
#define BALL_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

typedef struct
{
	dl_u8 enabled;
	dl_u8 state;	// 0 - inactive
				// 1 - resetting?
				// 2 - active
				// 0xff - dying?
	dl_u16 x; // high resolution position 256 pixels, 256 subpixels
	dl_u16 y; // high resolution position 256 pixels, 256 subpixels
	dl_u16 speedx;
	dl_u16 speedy; // high resolution
	const dl_u8* currentSprite;
	const dl_u8* bitShiftedSprites;
	dl_u8 fallStateCounter;
} BallData;

void Ball_Init(BallData* ballData, dl_u8 roomNumber, const Resources* resources);
void Ball_Update(BallData* ballData, dl_u8* framebuffer, dl_u8* cleanBackground);

#endif