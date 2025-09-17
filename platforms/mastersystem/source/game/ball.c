#include "ball.h"
#include "draw_utils.h"
#include "physics_utils.h"
#include "dl_platform.h"

dl_u8 ballData_enabled;
dl_u8 ballData_state;	// 0 - inactive
						// 1 - resetting?
						// 2 - active
						// 0xff - dying?
dl_u16 ballData_x; // high resolution position 256 pixels, 256 subpixels
dl_u16 ballData_y; // high resolution position 256 pixels, 256 subpixels
dl_u16 ballData_speedx;
dl_u16 ballData_speedy; // high resolution
dl_u8 ballData_fallStateCounter;

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

void initBallPhysics(void)
{
	ballData_state = BALL_ACTIVE;
	ballData_x = SET_HIGH_BYTE(BALL_START_X);
	ballData_y = SET_HIGH_BYTE(BALL_START_Y);
	ballData_speedx = 0xffa8;
	ballData_speedy = 0;
}

void Ball_Init(dl_u8 roomNumber, const Resources* resources)
{
	const dl_u8* roomsWithBouncingBall;

	ballData_state = BALL_INACTIVE;
	ballData_enabled = FALSE;

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

	ballData_enabled = TRUE;
	ballData_fallStateCounter = 0; // unsure if should init every reset or just at room start

	initBallPhysics();
}


void Ball_Update(void)
{
	dl_u8 terrainTest;

	if (!ballData_enabled)
		return;

	// we're deactivated, so activate
	if (!ballData_state)
	{
		initBallPhysics();
		return;
	}

	if ((dl_s8)ballData_fallStateCounter > 0)
	{
		// jumping up
		ballData_speedy += 0xa;
		ballData_fallStateCounter--;

		if (!ballData_fallStateCounter)
		{
			ballData_speedy = 0;
		}
	}
	else if ((dl_s8)ballData_fallStateCounter < 0)
	{
		// on ground
		ballData_fallStateCounter++;

		if (!ballData_fallStateCounter)
		{
			ballData_speedy = 0xff00;
			ballData_fallStateCounter = 0xa;
		}
	}
	else
	{
		if (TOUCHES_TERRAIN(testTerrainCollision(ballData_x, 
												 ballData_y, 
												 BALL_SPRITE_ROWS, 
												 ballGroundCollisionMasks)))
		{
			// we've hit the ground. stop our y speed
			// and set the counter so that we don't move
			// for a bit.
			ballData_speedy = 0xff00;
			ballData_fallStateCounter = BALL_GROUND_FREEZE_TIME; 
		}
		else
		{
			// continue falling
			ballData_speedy += 0x12;

			if (ballData_speedy > 0x100)
				ballData_speedy = 0x100;
		}
	}
	
	if ((dl_s8)ballData_fallStateCounter >= 0)
	{
		ballData_x += ballData_speedx;
		ballData_y += ballData_speedy;	

		terrainTest = testTerrainCollision(ballData_x, 
										   ballData_y, 
										   BALL_WALL_SENSOR_YOFFSET, 
										   ballWideCollisionMasks);

		if (TOUCHES_TERRAIN(terrainTest))
		{
			ballData_state = 0xff;
		}
	}

	if (ballData_state == 0xff)
	{
		ballData_state = 0;
		return;
	}
}

void Ball_Draw(void)
{
	if (ballData_enabled)
	{
		SMS_addTwoAdjoiningSprites((ballData_x >> 8) << 1, 
									ballData_y >> 8, 
									2 * ((dl_s8)ballData_fallStateCounter < 0));
	}
}