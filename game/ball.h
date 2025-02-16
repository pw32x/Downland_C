#ifndef BALL_INCLUDE_H
#define BALL_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

#define BALL_INACTIVE			0
#define BALL_RESETTING_MAYBE	1
#define BALL_ACTIVE				2
#define BALL_DYING_MAYBE		0xff

#define BALL_START_X 0x65 // 101
#define BALL_START_Y 0x74 // 116

typedef struct
{
	u8 state;	// 0 - inactive
				// 1 - resetting?
				// 2 - active
				// 0xff - dying?
	u8 x;
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u8 speedx;
	u16 speedy; // high resolution
	u16 framebufferDrawLocation;
	u16 previousFramebufferDrawLocation;
	u8* currentSprite;
	u8* previousSprite;
	u8* spriteData;
} BallData;

void Ball_Init(BallData* ballData, u8* ballSpriteData, u8* roomsWithBouncingBall);
void Ball_Update(BallData* ballData, u8* framebuffer);

#endif