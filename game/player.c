#include "player.h"

#include "base_defines.h"
#include "draw_utils.h"
#include "physics_utils.h"
#include "debug_utils.h"

// all the states are mutually exclusive
#define PLAYER_STATE_STAND		0
#define PLAYER_STATE_RUN		1
#define PLAYER_STATE_JUMP		2
#define PLAYER_STATE_FALL		3
#define PLAYER_STATE_CLIMB		4


#define PLAYER_RUN_SPEED_LEFT	0xffca
#define PLAYER_RUN_SPEED_RIGHT	0x36

#define PLAYER_JUMP_SPEED		0xff61
#define PLAYER_MAX_FALL_SPEED	0x100
#define PLAYER_JUMP_AIR_COUNT	0x28

#define PLAYER_CLIMB_UP_SPEED	0xffc0
#define PLAYER_CLIMB_DOWN_SPEED 0x70

#define PLAYER_FACING_LEFT		0
#define PLAYER_FACING_RIGHT		0xff

#define PLAYER_START_X 0x70 // 112
#define PLAYER_START_Y 0xa5 // 165

#define PLAYER_WALL_SENSOR_YOFFSET		12
#define PLAYER_GROUND_SENSOR_YOFFSET	16
#define PLAYER_ROPE_SENSOR_YOFFSET		8

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
	0x03c0, // 0000001111000000b
    0x00f0, // 0000000011110000b
    0x003c, // 0000000000111100b
    0x0f00, // 0000111100000000b
};

u16 ropeCollisionMasks[4] = 
{
    0x0300, // 0000001100000000b
    0x00c0, // 0000000011000000b
    0x0030, // 0000000000110000b
    0x0c00, // 0000110000000000b
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
	playerData->ignoreRopesCounter = 0;

	playerData->bitShiftedSprites = resources->bitShiftedSprites_player;
	playerData->bitShiftedCollisionMasks = resources->bitShiftedCollisionmasks_player;

	playerData->currentSpriteNumber = computeSpriteNumber(playerData->facingDirection, playerData->currentFrameNumber);

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
											        playerData->currentSpriteNumber,
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
			playerData->speedx = 0;
		}

		playerData->currentFrameNumber = (playerData->globalAnimationCounter >> 2) & 0x3;

		// if still in run, check for falling
		if (!TOUCHES_TERRAIN(testTerrainCollision(playerData->x, 
												  playerData->y, 
												  PLAYER_GROUND_SENSOR_YOFFSET, 
												  playerGroundCollisionMasks,
												  cleanBackground)))
		{
			playerData->state = PLAYER_STATE_FALL;
		}
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

		if (TOUCHES_TERRAIN(testTerrainCollision(playerData->x, 
												 playerData->y, 
												 PLAYER_GROUND_SENSOR_YOFFSET, 
												 playerGroundCollisionMasks,
												 cleanBackground)))
		{
			playerData->state = PLAYER_STATE_STAND;
			playerData->speedy = 0;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
		}


	}
	else if (playerData->state == PLAYER_STATE_CLIMB)
	{
		playerData->speedy = 0;
		u8 testResult = testTerrainCollision(playerData->x, 
											 playerData->y, 
											 PLAYER_ROPE_SENSOR_YOFFSET, 
											 ropeCollisionMasks,
											 cleanBackground);

		if (joystickState->jumpPressed)
		{
			// apply side movement if a direction was held
			if (joystickState->leftDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
				playerData->speedy = 0xff61;
				playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
				playerData->state = PLAYER_STATE_JUMP;
				playerData->ignoreRopesCounter = 20;
			}
			else if (joystickState->rightDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
				playerData->facingDirection = PLAYER_FACING_RIGHT;
				playerData->speedy = 0xff61;
				playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
				playerData->state = PLAYER_STATE_JUMP;
				playerData->ignoreRopesCounter = 20;
			}
		}
		else if (joystickState->upDown)
		{
			playerData->speedy = PLAYER_CLIMB_UP_SPEED;

			playerData->currentFrameNumber = ((playerData->globalAnimationCounter >> 3) & 0x1) + 4;

			if (!TOUCHES_VINE(testResult))
			{
				playerData->speedy = 0;
				playerData->currentFrameNumber = PLAYER_CLIMB_FRAME_0;
			}
		}	
		else if (joystickState->downDown)
		{
			playerData->speedy = PLAYER_CLIMB_DOWN_SPEED;
			playerData->currentFrameNumber = ((playerData->globalAnimationCounter >> 3) & 0x1) + 4;

			if (!TOUCHES_VINE(testResult))
			{
				playerData->state = PLAYER_STATE_FALL;
			}
		}
	}

	playerData->x += playerData->speedx;
	playerData->y += playerData->speedy;




	if (playerData->ignoreRopesCounter)
		playerData->ignoreRopesCounter--;

	/*
    //debugSetPixel((GET_HIGH_BYTE(playerData->x) << 1), 
    //              GET_HIGH_BYTE(playerData->y) + PLAYER_ROPE_SENSOR_YOFFSET, 0xff00ff00);

	debugDrawBox(((GET_HIGH_BYTE(playerData->x) << 1) / 8) * 8, 
			     GET_HIGH_BYTE(playerData->y), 
			     16, 
			     16, 
			     0xff00ff00);

	extern u8 leftPixelData;
	extern u8 rightPixelData;


	getTerrainValue(playerData->x, 
					playerData->y, 
					PLAYER_ROPE_SENSOR_YOFFSET, 
					ropeCollisionMasks,
					cleanBackground);

	u8 copyLeftPixelData = leftPixelData;
	u8 copyRightPixelData = rightPixelData;

	for (int loop = 0; loop < 8; loop++)
	{
		debugSetPixel((GET_HIGH_BYTE(playerData->x) << 1) + (7 - loop), 
                      GET_HIGH_BYTE(playerData->y) + PLAYER_ROPE_SENSOR_YOFFSET, 
					  copyLeftPixelData & 0x1 ? 0xffff0000 : 0x00000000);

		debugSetPixel((GET_HIGH_BYTE(playerData->x) << 1) + (15 - loop), 
                      GET_HIGH_BYTE(playerData->y) + PLAYER_ROPE_SENSOR_YOFFSET, 
					  copyRightPixelData & 0x1 ? 0xffff0000 : 0x00000000);

		copyLeftPixelData >>= 1;
		copyRightPixelData >>= 1;
	}
	*/

	/*
	getTerrainValue(playerData->x, 
					playerData->y, 
					PLAYER_ROPE_SENSOR_YOFFSET, 
					ropeCollisionMasks,
					cleanBackground);
	*/

	// if in the air, check for ropes
	if ((playerData->state == PLAYER_STATE_JUMP ||
		playerData->state == PLAYER_STATE_FALL) &&
		!playerData->ignoreRopesCounter)
	{
		u8 testResult = testTerrainCollision(playerData->x, 
											 playerData->y, 
											 PLAYER_ROPE_SENSOR_YOFFSET, 
											 ropeCollisionMasks,
											 cleanBackground);
		if (TOUCHES_VINE(testResult))
		{
			playerData->state = PLAYER_STATE_CLIMB;
			playerData->speedx = 0;
			playerData->speedy = 0;
			playerData->currentFrameNumber = PLAYER_CLIMB_FRAME_0;
			playerData->jumpAirCounter = 0;
		}
	}

	



	// wall detection when moving
	if (playerData->speedx && 
		TOUCHES_TERRAIN(testTerrainCollision(playerData->x, 
						playerData->y, 
						PLAYER_WALL_SENSOR_YOFFSET, 
						playerGroundCollisionMasks,
						cleanBackground)))
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

	playerData->currentSpriteNumber = computeSpriteNumber(playerData->facingDirection, playerData->currentFrameNumber);

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
												    playerData->currentSpriteNumber,
												    GET_HIGH_BYTE(playerData->x) & 3, 
												    PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	//if (playerData->state == PLAYER_STATE_CLIMB)
	//	return;

	drawSprite_24PixelsWide(playerData->currentSprite, 
							GET_HIGH_BYTE(playerData->x), 
							GET_HIGH_BYTE(playerData->y), 
							PLAYER_SPRITE_ROWS, 
							framebuffer);
}

u8 collisionTestBuffer[PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE];
u8 utilityBuffer[PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE];

u8 Player_HasCollision(PlayerData* playerData, u8* framebuffer, u8* cleanBackground)
{
	u8 sensorX = GET_HIGH_BYTE(playerData->x);

	u8* currentCollisionMask = getBitShiftedSprite(playerData->bitShiftedCollisionMasks, 
												   playerData->currentSpriteNumber,
												   sensorX & 3, 
												   PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE);

	u8 sensorY = GET_HIGH_BYTE(playerData->y) + 1; // start one line down
	u16 location = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

	framebuffer = &framebuffer[location];
	u8* cleanBackgroundRunner = &cleanBackground[location];

	u8* collisionTestBufferRunner = collisionTestBuffer;


	for (u8 loop = 0; loop < PLAYER_COLLISION_MASK_ROWS; loop++)
	{
		collisionTestBufferRunner[0] = (framebuffer[0] & currentCollisionMask[0]) | cleanBackgroundRunner[0];
		collisionTestBufferRunner[1] = (framebuffer[1] & currentCollisionMask[1]) | cleanBackgroundRunner[1];
		collisionTestBufferRunner[2] = (framebuffer[2] & currentCollisionMask[2]) | cleanBackgroundRunner[2];


		// move along buffers
		framebuffer += FRAMEBUFFER_PITCH * 3;
		cleanBackgroundRunner += FRAMEBUFFER_PITCH * 3;
		collisionTestBufferRunner += 3;
		currentCollisionMask += 3;
	}

	u8* currentSprite = playerData->currentSprite + 3; // start one line down
	cleanBackgroundRunner = &cleanBackground[location];
	collisionTestBufferRunner = collisionTestBuffer;

	for (u8 loop = 0; loop < PLAYER_COLLISION_MASK_ROWS; loop++)
	{
		if ((collisionTestBufferRunner[0] != (currentSprite[0] | cleanBackgroundRunner[0])) ||
		    (collisionTestBufferRunner[1] != (currentSprite[1] | cleanBackgroundRunner[1])) ||
		    (collisionTestBufferRunner[2] != (currentSprite[2] | cleanBackgroundRunner[2])))
		{
			return TRUE;
		}

		cleanBackgroundRunner += FRAMEBUFFER_PITCH * 3;
		collisionTestBufferRunner += 3;
		currentSprite += 3 * 3;
	}

	return FALSE;
}