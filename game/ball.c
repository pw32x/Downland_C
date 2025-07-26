#include "ball.h"
#include "draw_utils.h"
#include "debug_utils.h"
#include "physics_utils.h"



#define BALL_START_X 0x65 // 101
#define BALL_START_Y 0x74 // 116

#define BALL_WALL_SENSOR_YOFFSET	5

#define BALL_GROUND_FREEZE_TIME 0xfb

#define BITSHIFTED_SPRITE_FRAME_SIZE (BALL_SPRITE_ROWS * 3) // rows * 3 bytes per row

// used as a sensor at the bottom of the
// ball to detect whether the ball is touching
// the floor. 
dl_u16 ballGroundCollisionMasks[4] =
{
	0x0300, // 0000001100000000b
	0x00c0, // 0000000011000000b
	0x0030, // 0000000000110000b
	0x0c00  // 0000110000000000b
};

// used as a sensor mid-way across the
// ball to detect whether it has touched
// a wall.
dl_u16 ballWideCollisionMasks[4] =
{
    0x3ff0, // 0011111111110000b
    0x0ffc, // 0000111111111100b
    0x03ff, // 0000001111111111b
    0xffc0, // 1111111111000000b
};

void initBallPhysics(BallData* ballData)
{
	ballData->state = BALL_RESETTING_MAYBE;
	ballData->x = SET_HIGH_BYTE(BALL_START_X);
	ballData->y = SET_HIGH_BYTE(BALL_START_Y);
	ballData->speedx = 0xffa8;
	ballData->speedy = 0;

	ballData->currentSprite = getBitShiftedSprite(ballData->bitShiftedSprites, 
											      0,
											      BALL_START_X & 3,
											      BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Ball_Init(BallData* ballData, dl_u8 roomNumber, const Resources* resources)
{
	const dl_u8* roomsWithBouncingBall;

	ballData->state = BALL_INACTIVE;
	ballData->enabled = FALSE;

	// check if this room uses the ball. if not, then return and
	// stay disabled.
	roomsWithBouncingBall = resources->roomsWithBouncingBall;

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


void Ball_Update(BallData* ballData, dl_u8* framebuffer, dl_u8* cleanBackground)
{
	dl_u8 terrainTest;

	if (!ballData->enabled)
		return;

	// we're deactivated, so activate
	if (!ballData->state)
	{
		initBallPhysics(ballData);
		return;
	}

	if ((dl_s8)ballData->fallStateCounter > 0)
	{
		// jumping up
		ballData->speedy += 0xa;
		ballData->fallStateCounter--;

		if (!ballData->fallStateCounter)
		{
			ballData->speedy = 0;
		}
	}
	else if ((dl_s8)ballData->fallStateCounter < 0)
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
		if (TOUCHES_TERRAIN(testTerrainCollision(ballData->x, 
												 ballData->y, 
												 BALL_SPRITE_ROWS, 
												 ballGroundCollisionMasks, 
												 cleanBackground)))
		{
			// we've hit the ground. stop our y speed
			// and set the counter so that we don't move
			// for a bit.
			ballData->speedy = 0xff00;
			ballData->fallStateCounter = BALL_GROUND_FREEZE_TIME; 
		}
		else
		{
			// continue falling
			ballData->speedy += 0x12;

			if (ballData->speedy > 0x100)
				ballData->speedy = 0x100;
		}
	}
	
	eraseSprite_24PixelsWide(ballData->currentSprite,
							 GET_HIGH_BYTE(ballData->x),
							 GET_HIGH_BYTE(ballData->y),
							 BALL_SPRITE_ROWS,
							 framebuffer, 
							 cleanBackground);

	if ((dl_s8)ballData->fallStateCounter >= 0)
	{
		ballData->x += ballData->speedx;
		ballData->y += ballData->speedy;	

		terrainTest = testTerrainCollision(ballData->x, 
										   ballData->y, 
										   BALL_WALL_SENSOR_YOFFSET, 
										   ballWideCollisionMasks, 
										   cleanBackground);

		if (TOUCHES_TERRAIN(terrainTest))
		{
			ballData->state = 0xff;
		}
	}

	ballData->currentSprite = getBitShiftedSprite(ballData->bitShiftedSprites, 
												  ((dl_s8)ballData->fallStateCounter < 0), // sprite 0 (not squished) if fallStateCounterSigned >= 0, else sprite 1 (squished)
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
							BALL_SPRITE_ROWS, 
							framebuffer);
}
