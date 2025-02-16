#include "ball.h"
#include "draw_utils.h"

u8 collisionCheckXOffsets[4] = { 0, 0, 0, 1 };

u16 ballGroundCollisionMaskData[4] =
{
	0x0300, // 0000001100000000b
	0x00c0, // 0000000011000000b
	0x0030, // 0000000000110000b
	0x0c00  // 0000110000000000b
};


void initBallPhysics(BallData* ballData)
{
	ballData->state = BALL_RESETTING_MAYBE;
	ballData->x = SET_HIRES(BALL_START_X);
	ballData->y = SET_HIRES(BALL_START_Y);
	ballData->speedx = 0xff80;
	ballData->speedy = 0;
}

void Ball_Init(BallData* ballData, u8* ballSpriteData, u8 roomNumber, u8* roomsWithBouncingBall)
{
	ballData->state = BALL_INACTIVE;
	ballData->enabled = FALSE;

	while (*roomsWithBouncingBall != roomNumber && *roomsWithBouncingBall != 0xff)
	{
		roomsWithBouncingBall++;
	}

	if (*roomsWithBouncingBall == 0xff)
	{
		return;
	}

	ballData->enabled = TRUE;
	ballData->sprite1 = ballSpriteData;
	ballData->sprite2 = ballSpriteData + (BALL_SPRITE_ROWS * 2); // 2 bytes per row
	ballData->currentSprite = ballData->sprite1;

	ballData->fallStateCounter = 0; // unsure if should init every reset or just at room start

	initBallPhysics(ballData);
}

void Ball_Update(BallData* ballData, u8* framebuffer, u8* cleanBackground)
{
	if (!ballData->enabled)
		return;

	if (!ballData->state)
	{
		initBallPhysics(ballData);
	}

	eraseSprite_16PixelsWide(framebuffer, 
							 cleanBackground,
							 GET_FROM_HIRES(ballData->x),
							 GET_FROM_HIRES(ballData->y),
							 ballData->currentSprite,
							 BALL_SPRITE_ROWS);

	if (ballData->fallStateCounter > 0)
	{
		// jumping up
		ballData->currentSprite = ballData->sprite1;
		ballData->speedy += 0xa;
		ballData->fallStateCounter--;

		if (!ballData->fallStateCounter)
		{
			ballData->speedy = 0;
		}
	}
	else if (ballData->fallStateCounter < 0)
	{
		// on ground
		ballData->currentSprite = ballData->sprite2;
		ballData->fallStateCounter++;

		if (!ballData->fallStateCounter)
		{
			ballData->speedy = 0xff00;
			ballData->fallStateCounter = 0xa;
		}
	}
	else
	{
		// falling
		u8 pixelX = GET_HIGH_BYTE(ballData->x);
		u8 tableIndex = pixelX & 0x3;
		u8 collectionCheckXOffset = collisionCheckXOffsets[tableIndex]; // offset the x byte position depending on x pixel position
		u16 ballGroundCollisionMask = ballGroundCollisionMaskData[tableIndex]; // different masks for different x pixel positions

		u8 sensorX = pixelX + BALL_SPRITE_ROWS + collectionCheckXOffset;

		u16 framebufferPosition = GET_FRAMEBUFFER_LOCATION(sensorX, GET_FROM_HIRES(ballData->y));

		// if not hitting anything, just keep falling
		if ((framebuffer[framebufferPosition] & GET_HIGH_BYTE(ballGroundCollisionMask)) == 0 &&
			(framebuffer[framebufferPosition + 1] & GET_LOW_BYTE(ballGroundCollisionMask)) == 0)
		{
			ballData->speedy += 0x12;

			if (ballData->speedy > 0x100)
				ballData->speedy = 0x100;
		}
		else
		{
			// perform terrain collision check

			// assume we've hit something
			ballData->speedy = 0xff00;
			ballData->fallStateCounter = 0xfb;

			// change the sprite to squished
			ballData->currentSprite = ballData->sprite2;
		}
	}



	ballData->x += ballData->speedx;
	ballData->y += ballData->speedy;

	// check for a wall

	// draw
	drawSprite_16PixelsWide(ballData->currentSprite, 
							GET_FROM_HIRES(ballData->x),
							GET_FROM_HIRES(ballData->y),
							BALL_SPRITE_ROWS,
							framebuffer);
}
