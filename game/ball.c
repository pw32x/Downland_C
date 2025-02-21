#include "ball.h"
#include "draw_utils.h"
#include "debug_utils.h"

#define BALL_INACTIVE			0
#define BALL_RESETTING_MAYBE	1
#define BALL_ACTIVE				2
#define BALL_DYING_MAYBE		0xff

#define BALL_START_X 0x65 // 101
#define BALL_START_Y 0x74 // 116

u8 collisionCheckXOffsets[4] = { 0, 0, 0, 1 };

#define BITSHIFTED_SPRITE_FRAME_SIZE (BALL_SPRITE_ROWS * 3) // rows * 3 bytes per row

u16 ballGroundCollisionMasks[4] =
{
	0x0300, // 0000001100000000b
	0x00c0, // 0000000011000000b
	0x0030, // 0000000000110000b
	0x0c00  // 0000110000000000b
};

u16 ballWideCollisionMasks[4] =
{
    0x3ff0, // 0011111111110000b
    0x0ffc, // 0000111111111100b
    0x03ff, // 0000001111111111b
    0xffc0, // 1111111111000000b
};

u8* getCurrentSprite(u8* spriteData, u8 frameNumber, u8 x, u8 spriteFrameSize)
{
	// x will be 0 to 3
	return spriteData + (frameNumber * (spriteFrameSize * 4)) + (x * spriteFrameSize);
}

void initBallPhysics(BallData* ballData)
{
	ballData->state = BALL_RESETTING_MAYBE;
	ballData->x = SET_HIGH_BYTE(BALL_START_X);
	ballData->y = SET_HIGH_BYTE(BALL_START_Y);
	ballData->speedx = 0xffa8;
	ballData->speedy = 0;

	ballData->currentSprite = getCurrentSprite(ballData->bitShiftedSprites, 
											   0,
											   BALL_START_X & 3,
											   BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Ball_Init(BallData* ballData, u8 roomNumber, Resources* resources)
{
	ballData->state = BALL_INACTIVE;
	ballData->enabled = FALSE;

	// check if this room uses the ball. if not, then return and
	// stay disabled.
	const u8* roomsWithBouncingBall = resources->roomsWithBouncingBall;

	while (*roomsWithBouncingBall != roomNumber && *roomsWithBouncingBall != 0xff)
	{
		roomsWithBouncingBall++;
	}

	if (*roomsWithBouncingBall == 0xff)
	{
		return;
	}

	ballData->enabled = TRUE;
	ballData->fallStateCounter = 0; // unsure if should init every reset or just at room start
	ballData->bitShiftedSprites = resources->bitShiftedSprites_bouncyBall;

	initBallPhysics(ballData);
}


void Ball_Update(BallData* ballData, u8* framebuffer, u8* cleanBackground)
{
	if (!ballData->enabled)
		return;

	if (!ballData->state)
	{
		initBallPhysics(ballData);
		return;
	}

	if ((s8)ballData->fallStateCounter > 0)
	{
		// jumping up
		ballData->speedy += 0xa;
		ballData->fallStateCounter--;

		if (!ballData->fallStateCounter)
		{
			ballData->speedy = 0;
		}
	}
	else if ((s8)ballData->fallStateCounter < 0)
	{
		// on ground
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
		u16 ballGroundCollisionMask = ballGroundCollisionMasks[tableIndex]; // different masks for different x pixel positions

		u8 sensorX = pixelX + collectionCheckXOffset;
		u8 sensorY = GET_HIGH_BYTE(ballData->y) + BALL_SPRITE_ROWS;

		u16 framebufferPosition = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

		// if not hitting anything, just keep falling
		if ((cleanBackground[framebufferPosition] & GET_HIGH_BYTE(ballGroundCollisionMask)) == 0 &&
			(cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(ballGroundCollisionMask)) == 0)
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
		}
	}
	
	eraseSprite_24PixelsWide(framebuffer, 
							 cleanBackground,
							 GET_HIGH_BYTE(ballData->x),
							 GET_HIGH_BYTE(ballData->y),
							 ballData->currentSprite,
							 BALL_SPRITE_ROWS);

	if ((s8)ballData->fallStateCounter >= 0)
	{
		ballData->x += ballData->speedx;
		ballData->y += ballData->speedy;	

		// check again for collision, but for a wall
		u8 pixelX = GET_HIGH_BYTE(ballData->x);
		u8 tableIndex = pixelX & 0x3;
		u8 collectionCheckXOffset = collisionCheckXOffsets[tableIndex]; // offset the x byte position depending on x pixel position
		u16 ballWideCollisionMask = ballWideCollisionMasks[tableIndex]; // different masks for different x pixel positions

		u8 sensorX = pixelX + collectionCheckXOffset;
		u8 sensorY = GET_HIGH_BYTE(ballData->y) + BALL_WALL_SENSOR_YOFFSET;

		u16 framebufferPosition = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

		// if hitting something, reset
		if ((cleanBackground[framebufferPosition] & GET_HIGH_BYTE(ballWideCollisionMask)) != 0 ||
			(cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(ballWideCollisionMask)) != 0)
		{
			ballData->state = 0xff;
		}
	}

	ballData->currentSprite = getCurrentSprite(ballData->bitShiftedSprites, 
											   ((s8)ballData->fallStateCounter < 0), // sprite 0 (not squished) if fallStateCounterSigned >= 0, else sprite 1 (squished)
											   GET_HIGH_BYTE(ballData->x) & 3, 
											   BITSHIFTED_SPRITE_FRAME_SIZE);

	if (ballData->state == 0xff)
	{
		ballData->state = 0;
		return;
	}

	drawSprite_24PixelsWide(ballData->currentSprite, 
							GET_HIGH_BYTE(ballData->x), 
							GET_HIGH_BYTE(ballData->y), 
							BALL_SPRITE_ROWS, framebuffer);
}
