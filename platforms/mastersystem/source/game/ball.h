#ifndef BALL_INCLUDE_H
#define BALL_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

#define BALL_INACTIVE			0
#define BALL_ACTIVE				1
#define BALL_DYING_MAYBE		0xff

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
	dl_u8 fallStateCounter;
} BallData;

void Ball_Init(BallData* ballData, dl_u8 roomNumber, const Resources* resources);
void Ball_Update(BallData* ballData, dl_u8* cleanBackground);

#endif