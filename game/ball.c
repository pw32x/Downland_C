#include "ball.h"

void Ball_Init(BallData* ballData, u8* ballSpriteData, u8* roomsWithBouncingBall)
{
	ballData->state = BALL_INACTIVE;
	ballData->x = BALL_START_X;
	ballData->y = SET_HIRES(BALL_START_Y);
	ballData->spriteData = ballSpriteData;
}

void Ball_Update(BallData* ballData, u8* framebuffer)
{
	if (!ballData->state)
		return;
}
