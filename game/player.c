#include "player.h"

#include "base_defines.h"
#include "draw_utils.h"
#include "physics_utils.h"

// all the states are mutually exclusive
#define PLAYER_STATE_STAND		0
#define PLAYER_STATE_RUN		1
#define PLAYER_STATE_JUMP		2
#define PLAYER_STATE_FALL		3


#define PLAYER_RUN_SPEED_LEFT	0xffca
#define PLAYER_RUN_SPEED_RIGHT	0x36

#define PLAYER_JUMP_SPEED		0xff61
#define PLAYER_MAX_FALL_SPEED	0x100
#define PLAYER_JUMP_AIR_COUNT	0x28

#define PLAYER_FACING_LEFT		0
#define PLAYER_FACING_RIGHT		0xff

#define PLAYER_START_X 0x70 // 112
#define PLAYER_START_Y 0xa5 // 165

#define PLAYER_WALL_SENSOR_YOFFSET 12
#define PLAYER_GROUND_SENSOR_YOFFSET 16

#define PLAYER_SPRITE_RIGHT_STAND		0
#define PLAYER_SPRITE_RIGHT_RUN0		1
#define PLAYER_SPRITE_RIGHT_RUN1_JUMP	2
#define PLAYER_SPRITE_RIGHT_RUN2		3
#define PLAYER_SPRITE_RIGHT_CLIMB		4
#define PLAYER_SPRITE_LEFT_CLIMB		5
#define PLAYER_SPRITE_LEFT_STAND		6
#define PLAYER_SPRITE_LEFT_RUN0			7
#define PLAYER_SPRITE_LEFT_RUN1_JUMP	8
#define PLAYER_SPRITE_LEFT_RUN2			9

#define PLAYER_RUN_FRAME_0_STAND		0
#define PLAYER_RUN_FRAME_1				1
#define PLAYER_RUN_FRAME_2_JUMP			2
#define PLAYER_RUN_FRAME_3				3
#define PLAYER_CLIMB_FRAME_0			4
#define PLAYER_CLIMB_FRAME_1			5
#define PLAYER_FRAME_COUNT				6

u16 playerGroundCollisionMasks[4] =
{
	0x03c0, //  0000001111000000b
    0x00f0, //  0000000011110000b
    0x003c, //  0000000000111100b
    0x0f00, //  0000111100000000b
};

u8 computeSpriteNumber(u8 facingDirection, u8 currentFrameNumber)
{
	// if in run/stand, then compute the sprite based on direction
	if (currentFrameNumber <= PLAYER_RUN_FRAME_3)
	{
		// left is 0, right is 1.
		return facingDirection ? currentFrameNumber : currentFrameNumber + PLAYER_FRAME_COUNT;
	}
		
	// if climbing, then just return because frame number and
	// sprite numbers match for that.
	return currentFrameNumber;
}

void Player_Init(PlayerData* playerData, const Resources* resources)
{
	playerData->state = PLAYER_STATE_STAND;
	playerData->x = SET_HIGH_BYTE(PLAYER_START_X);
	playerData->y = SET_HIGH_BYTE(PLAYER_START_Y);
	playerData->speedx = 0xffa8;
	playerData->speedy = 0;
	playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
	playerData->facingDirection = PLAYER_FACING_LEFT;
	playerData->safeLanding = TRUE;

	playerData->bitShiftedSprites = resources->bitShiftedSprites_player;

	u8 spriteNumber = computeSpriteNumber(playerData->facingDirection, playerData->currentFrameNumber);

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
											        spriteNumber,
											        PLAYER_START_X & 3,
											        PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Player_Update(PlayerData* playerData, 
				   const JoystickState* joystickState, 
				   u8* framebuffer, 
				   u8* cleanBackground)
{
	eraseSprite_24PixelsWide(framebuffer, 
							 cleanBackground,
							 GET_HIGH_BYTE(playerData->x),
							 GET_HIGH_BYTE(playerData->y),
							 playerData->currentSprite,
							 PLAYER_SPRITE_ROWS);

	playerData->globalAnimationCounter++;

	if (playerData->state == PLAYER_STATE_STAND)
	{
		playerData->speedx = 0;
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;

		if (joystickState->jumpPressed)
		{
			playerData->speedy = 0xff61;
			playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
			playerData->state = PLAYER_STATE_JUMP;

			// apply side movement if a direction was held
			if (joystickState->leftDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
			}
			else if (joystickState->rightDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
				playerData->facingDirection = PLAYER_FACING_RIGHT;
			}
		}
		else if (joystickState->leftDown ||
				 joystickState->rightDown)
		{
			playerData->state = PLAYER_STATE_RUN;
		}		
	}

	if (playerData->state == PLAYER_STATE_RUN)
	{
		if (joystickState->jumpPressed)
		{
			playerData->speedy = 0xff61;
			playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
			playerData->state = PLAYER_STATE_JUMP;
		}
		else if (joystickState->leftDown)
		{
			playerData->speedx = PLAYER_RUN_SPEED_LEFT;
			playerData->facingDirection = PLAYER_FACING_LEFT;
		}
		else if (joystickState->rightDown)
		{
			playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
			playerData->facingDirection = PLAYER_FACING_RIGHT;
		}
		else
		{
			playerData->state = PLAYER_STATE_STAND;
		}

		playerData->currentFrameNumber = (playerData->globalAnimationCounter >> 2) & 0x3;
	}

	if (playerData->state == PLAYER_STATE_JUMP)
	{
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->jumpAirCounter--;
		playerData->speedy += 3; // apply gravity
		if (!playerData->jumpAirCounter)
		{
			playerData->state = PLAYER_STATE_FALL;
			playerData->speedy = 0;
		}
	}
	else if (playerData->state == PLAYER_STATE_FALL)
	{
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->speedy += 6; // apply more gravity

		if (playerData->speedy > PLAYER_MAX_FALL_SPEED)
			playerData->speedy = PLAYER_MAX_FALL_SPEED;

		//// reduce x speed while falling by a little
		//if (playerData->speedx)
		//	playerData->facingDirection == PLAYER_FACING_LEFT ? playerData->speedx++ : playerData->speedx--;

		if (testCollision(playerData->x, 
						  playerData->y, 
						  PLAYER_GROUND_SENSOR_YOFFSET, 
						  playerGroundCollisionMasks,
						  cleanBackground))
		{
			playerData->state = PLAYER_STATE_STAND;
			playerData->speedy = 0;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
		}
	}

	playerData->x += playerData->speedx;
	playerData->y += playerData->speedy;

	// if still in run, check for falling
	if (playerData->state == PLAYER_STATE_RUN)
	{
		if (!testCollision(playerData->x, 
						   playerData->y, 
						   PLAYER_GROUND_SENSOR_YOFFSET, 
						   playerGroundCollisionMasks,
						   cleanBackground))
		{
			playerData->state = PLAYER_STATE_FALL;
		}
	}

	// wall detection when moving
	if (playerData->speedx && 
		testCollision(playerData->x, 
					   playerData->y, 
					   PLAYER_WALL_SENSOR_YOFFSET, 
					   playerGroundCollisionMasks,
					   cleanBackground))
	{
		playerData->x -= playerData->speedx;

		if (playerData->state == PLAYER_STATE_RUN)
		{
			playerData->speedx = 0;
			playerData->state = PLAYER_STATE_STAND;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
		}
		else if (playerData->state == PLAYER_STATE_JUMP)
		{
			playerData->speedx = -playerData->speedx;
			playerData->jumpAirCounter = 1;
			playerData->facingDirection = !playerData->facingDirection;
		}
		else if (playerData->state == PLAYER_STATE_FALL)
		{
			playerData->speedx = 0;
		}
	}

	u8 spriteNumber = computeSpriteNumber(playerData->facingDirection, playerData->currentFrameNumber);

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
												    spriteNumber,
												    GET_HIGH_BYTE(playerData->x) & 3, 
												    PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	drawSprite_24PixelsWide(playerData->currentSprite, 
							GET_HIGH_BYTE(playerData->x), 
							GET_HIGH_BYTE(playerData->y), 
							PLAYER_SPRITE_ROWS, 
							framebuffer);
}