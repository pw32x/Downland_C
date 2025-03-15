#include "player.h"

#include <memory.h>

#include "base_defines.h"
#include "draw_utils.h"
#include "physics_utils.h"
#include "debug_utils.h"
#include "game_types.h"
#include "pickup_types.h"
#include "door_utils.h"
#include "sound.h"

// all the states are mutually exclusive
#define PLAYER_STATE_STAND			0
#define PLAYER_STATE_RUN			1
#define PLAYER_STATE_JUMP			2
#define PLAYER_STATE_FALL			3
#define PLAYER_STATE_CLIMB			4
#define PLAYER_STATE_HANG_LEFT		5
#define PLAYER_STATE_HANG_RIGHT		6
#define PLAYER_STATE_REGENERATION	7
#define PLAYER_STATE_SPLAT			8
#define PLAYER_MIDAIR_DEATH			9
#define PLAYER_STATE_DEBUG			0xff

#define PLAYER_REGENERATION_TIME			0x190 // 400
#define PLAYER_REGENERATION_IMMOBILE_TIME	0x28  // 40
#define PLAYER_SPLAT_INITIAL_FREEZE_TIME	0xa
#define PLAYER_SPLAT_ANIMATION_TRIGGER_TIME 0x46
#define PLAYER_SPLAT_WAIT_TIME				(PLAYER_SPLAT_ANIMATION_TRIGGER_TIME + PLAYER_SPLAT_INITIAL_FREEZE_TIME)
#define PLAYER_MIDAIR_DEATH_PAUSE_TIME		0x32 // 40

#define PLAYER_RUN_SPEED_LEFT	0xffca
#define PLAYER_RUN_SPEED_RIGHT	0x36

#define PLAYER_DEBUG_SPEED 0xff

#define PLAYER_JUMP_SPEED		0xff61
#define PLAYER_MAX_FALL_SPEED	0x100
#define PLAYER_JUMP_AIR_COUNT	0x28

#define PLAYER_CLIMB_UP_SPEED	0xffc0
#define PLAYER_CLIMB_DOWN_SPEED 0x70

#define PLAYER_FACING_LEFT		0
#define PLAYER_FACING_RIGHT		0xff

#define PLAYER_ROPE_HOLD_COUNT	20

#define PLAYER_START_X 0x70 // 112
#define PLAYER_START_Y 0xa5 // 165

#define PLAYER_WALL_SENSOR_YOFFSET		12
#define PLAYER_GROUND_SENSOR_YOFFSET	16
#define PLAYER_ROPE_SENSOR_YOFFSET		8
#define PLAYER_OFF_ROPE_SENSOR_YOFFSET	7

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

void playerKill(PlayerData* playerData, u8* framebuffer, u8* cleanBackground)
{
	playerData->speedx = 0;
	playerData->speedy = 0;
	playerData->regenerationCounter = 0;
	playerData->isDead = TRUE;

	if (playerData->state == PLAYER_STATE_STAND ||
		playerData->state == PLAYER_STATE_RUN)
	{
		playerData->state = PLAYER_STATE_SPLAT;
		Sound_Stop(SOUND_RUN);
		Sound_Play(SOUND_SPLAT, FALSE);
		playerData->cantMoveCounter = PLAYER_SPLAT_WAIT_TIME;

		u8 x = GET_HIGH_BYTE(playerData->x);
		u8 y = GET_HIGH_BYTE(playerData->y);

		// erase the player, then draw the splat sprite
		// we won't redraw this again, like the original game
		eraseSprite_24PixelsWide_simple(x,
										y,
										PLAYER_SPRITE_ROWS,
										framebuffer, 
										cleanBackground);

		u8* splatSprite = getBitShiftedSprite(playerData->bitShiftedSplatSprite, 
											  0,
											  x & 3, 
											  PLAYER_SPLAT_SPRITE_FRAME_SIZE);

		drawSprite_24PixelsWide(splatSprite, 
								x, 
								y + 7, // draw the sprite to the ground
								PLAYER_SPLAT_SPRITE_ROWS, 
								framebuffer);


	}
	else
	{
		playerData->state = PLAYER_MIDAIR_DEATH;
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->cantMoveCounter = PLAYER_MIDAIR_DEATH_PAUSE_TIME;
		playerData->jumpAirCounter = 0;
		playerData->safeLanding = FALSE;
	}
}

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

void initPickups(RoomPickups roomPickups, 
				 const PickupPosition* roomPickupPositions,
				 const u8* keyPickUpDoorIndexes)
{
	// init all the keys for all the rooms for both players

	u8 pickUpTypes[NUM_PICKUPS_PER_ROOM];
	pickUpTypes[0] = PICKUP_TYPE_KEY; // the first two pickups are always keys
	pickUpTypes[1] = PICKUP_TYPE_KEY;

	for (int loop = 0; loop < NUM_ROOMS; loop++)
	{
		Pickup* pickups = roomPickups[loop];

		// the last three pickups are random between diamonds and money bags
		pickUpTypes[2] = rand() % 2;
		pickUpTypes[3] = rand() % 2;
		pickUpTypes[4] = rand() % 2;

		for (int innerLoop = 0; innerLoop < NUM_PICKUPS_PER_ROOM; innerLoop++)
		{
			pickups[innerLoop].type = pickUpTypes[innerLoop];
			pickups[innerLoop].state = INIT_PICKUP_STATE;
			pickups[innerLoop].x = roomPickupPositions->x;
			pickups[innerLoop].y = roomPickupPositions->y;

			// if the pickup is a key, get the index of the door it opens
			if (innerLoop < 2)
			{
				pickups[innerLoop].doorUnlockIndex = *keyPickUpDoorIndexes;
				keyPickUpDoorIndexes++;
			}
			else
			{
				pickups[innerLoop].doorUnlockIndex = 0;
			}			

			roomPickupPositions++;
		}
	}
}

void initDoors(u8* doorStateData, const u8* offsetsToDoorsAlreadyActivated)
{
	memset(doorStateData, 0, DOOR_TOTAL_COUNT);

	u8 alreadyOpenedState = 0x3; // set the two bits for each player

	while (*offsetsToDoorsAlreadyActivated != 0xff)
	{
		doorStateData[*offsetsToDoorsAlreadyActivated] = alreadyOpenedState;

		offsetsToDoorsAlreadyActivated++;
	}
}

void playerStartGameLoop(PlayerData* playerData, const Resources* resources)
{
	playerData->facingDirection = PLAYER_FACING_LEFT;

	u8* keyPickUpDoorIndexes = playerData->gameCompletionCount == 0 ? resources->keyPickUpDoorIndexes : resources->keyPickUpDoorIndexesHardMode;

	initPickups(playerData->gamePickups, 
				resources->roomPickupPositions,
				keyPickUpDoorIndexes);

	initDoors(playerData->doorStateData, 
			  resources->offsetsToDoorsAlreadyActivated);
}



void Player_CompleteGameLoop(PlayerData* playerData, const Resources* resources)
{
	playerData->gameCompletionCount++;
	playerStartGameLoop(playerData, resources);
}

void Player_GameInit(PlayerData* playerData, const Resources* resources)
{
	playerStartGameLoop(playerData, resources);

	playerData->lastDoor = NULL;
	playerData->lives = 3;
	playerData->isDead = 0;
	playerData->gameOver = FALSE;
	playerData->score = 0;
	playerData->gameCompletionCount = 0;

	playerData->bitShiftedSprites = resources->bitShiftedSprites_player;
	playerData->bitShiftedCollisionMasks = resources->bitShiftedCollisionmasks_player;
	playerData->bitShiftedSplatSprite = resources->bitShiftedSprites_playerSplat;

	// init timers
	for (int loop = 0; loop < NUM_ROOMS; loop++)
		playerData->roomTimers[loop] = ROOM_TIMER_DEFAULT;
}

void Player_RoomInit(PlayerData* playerData, const Resources* resources)
{
	// set initial state


	if (playerData->isDead)
	{
		Player_StartRegen(playerData);
		playerData->isDead = FALSE;
	}
	else if (playerData->lastDoor)
	{	
		if (playerData->state != PLAYER_STATE_DEBUG)
			playerData->state = PLAYER_STATE_STAND;

		// we've gone through a door
		playerData->x = SET_HIGH_BYTE(playerData->lastDoor->xLocationInNextRoom);
		playerData->y = SET_HIGH_BYTE(playerData->lastDoor->yLocationInNextRoom);
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
	}
	else
	{
		// game start
		playerData->state = PLAYER_STATE_STAND;
		Player_StartRegen(playerData);
		playerData->x = SET_HIGH_BYTE(PLAYER_START_X);
		playerData->y = SET_HIGH_BYTE(PLAYER_START_Y);
	}

	playerData->speedx = 0;
	playerData->speedy = 0;

	playerData->safeLanding = TRUE;
	playerData->ignoreRopesCounter = 0;

	playerData->currentSpriteNumber = computeSpriteNumber(playerData->facingDirection, playerData->currentFrameNumber);

	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
											        playerData->currentSpriteNumber,
											        PLAYER_START_X & 3,
											        PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);
}

void Player_StartRegen(PlayerData* playerData)
{
	playerData->state = PLAYER_STATE_REGENERATION;
	playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
	playerData->regenerationCounter = PLAYER_REGENERATION_TIME;
	playerData->cantMoveCounter = PLAYER_REGENERATION_IMMOBILE_TIME;
}

void Player_Update(PlayerData* playerData, 
				   const JoystickState* joystickState, 
				   u8* framebuffer, 
				   u8* cleanBackground,
				   DoorInfoData* doorInfoData,
				   u8* doorStateData)
{
	if (playerData->state == PLAYER_MIDAIR_DEATH)
	{
		if (playerData->cantMoveCounter)
			playerData->cantMoveCounter--;

		playerData->facingDirection = playerData->cantMoveCounter & 4 ? PLAYER_FACING_LEFT : PLAYER_FACING_RIGHT;

		if (!playerData->cantMoveCounter)
		{
			playerData->state = PLAYER_STATE_FALL;
		}
	}
	else if (playerData->state == PLAYER_STATE_SPLAT)
	{
		if (playerData->cantMoveCounter == PLAYER_SPLAT_ANIMATION_TRIGGER_TIME)
		{
			u8 x = GET_HIGH_BYTE(playerData->x);
			u8 y = GET_HIGH_BYTE(playerData->y);

			// erase the top 12 lines of the player sprite
			// to simulate animation of the splat sprite.
			eraseSprite_24PixelsWide_simple(x,
											y,
											12, 
											framebuffer, 
											cleanBackground);
		}

		if (playerData->cantMoveCounter)
			playerData->cantMoveCounter--;

		if (!playerData->cantMoveCounter)
		{
			if (playerData->lives)
			{
				playerData->lives--;
			}
			else
			{
				playerData->gameOver = TRUE;
			}
		}

		return;
	}

#ifdef DEV_MODE
	if (joystickState->debugStatePressed)
	{
		if (playerData->state != PLAYER_STATE_DEBUG)
		{
			playerData->state = PLAYER_STATE_DEBUG;
			playerData->speedy = 0;
			playerData->speedx = 0;
		}
		else
		{	
			playerData->regenerationCounter = 0;
			playerData->cantMoveCounter = 0;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
			playerData->state = PLAYER_STATE_JUMP;
			playerData->speedy = 0;
			playerData->jumpAirCounter = 1;
		}
	}
#endif

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		eraseSprite_24PixelsWide_simple(GET_HIGH_BYTE(playerData->x),
										GET_HIGH_BYTE(playerData->y),
										PLAYER_SPRITE_ROWS,
										framebuffer, 
										cleanBackground);
	}
	else
	{
		eraseSprite_24PixelsWide(playerData->currentSprite,
								 GET_HIGH_BYTE(playerData->x),
								 GET_HIGH_BYTE(playerData->y),
								 PLAYER_SPRITE_ROWS,
								 framebuffer, 
								 cleanBackground);
	}

	/*
	// to test the regeneration effect
	if (joystickState->jumpPressed)
	{
		playerData->state = PLAYER_STATE_REGENERATION;
		playerData->speedx = 0;
		playerData->speedy = 0;
		playerData->regenerationCounter = PLAYER_REGENERATION_TIME;
		playerData->cantMoveCounter = PLAYER_REGENERATION_IMMOBILE_TIME;
	}
	*/
	

	playerData->globalAnimationCounter++;

	if (playerData->state == PLAYER_STATE_DEBUG)
	{
		// apply side movement if a direction was held
		if (joystickState->leftDown)
		{
			playerData->x -= PLAYER_DEBUG_SPEED;
		}
		else if (joystickState->rightDown)
		{
			playerData->x += PLAYER_DEBUG_SPEED;
		}

		if (joystickState->upDown)
		{
			playerData->y -= PLAYER_DEBUG_SPEED;
		}	
		else if (joystickState->downDown)
		{
			playerData->y += PLAYER_DEBUG_SPEED;
		}
	}

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		if (joystickState->leftDown ||
			joystickState->rightDown ||
			joystickState->upDown ||
			joystickState->downDown)
		{
			playerData->regenerationCounter = 0;
		}

		if (playerData->regenerationCounter)
			playerData->regenerationCounter--;

		if (playerData->cantMoveCounter)
			playerData->cantMoveCounter--;

		if (!playerData->cantMoveCounter && 
			!playerData->regenerationCounter)
		{
			playerData->state = PLAYER_STATE_STAND;
			playerData->isDead = FALSE;
		}
	}

	if (playerData->state == PLAYER_STATE_STAND)
	{
		playerData->speedx = 0;
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;

		if (joystickState->jumpPressed)
		{
			playerData->speedy = 0xff61;
			playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
			playerData->state = PLAYER_STATE_JUMP;
			Sound_Play(SOUND_JUMP, FALSE);

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
			Sound_Play(SOUND_JUMP, FALSE);
			Sound_Stop(SOUND_RUN);
		}
		else if (joystickState->leftDown)
		{
			if (!playerData->speedx)
			{
				Sound_Play(SOUND_RUN, TRUE);
			}

			playerData->speedx = PLAYER_RUN_SPEED_LEFT;
			playerData->facingDirection = PLAYER_FACING_LEFT;
		}
		else if (joystickState->rightDown)
		{
			if (!playerData->speedx)
			{
				Sound_Play(SOUND_RUN, TRUE);
			}

			playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
			playerData->facingDirection = PLAYER_FACING_RIGHT;
		}
		else
		{
			playerData->state = PLAYER_STATE_STAND;
			playerData->speedx = 0;
			Sound_Stop(SOUND_RUN);
		}

		playerData->currentFrameNumber = (playerData->globalAnimationCounter >> 2) & 0x3;

		// if still in run, check for falling
		if (!TOUCHES_TERRAIN(testTerrainCollision(playerData->x, 
												  playerData->y, 
												  PLAYER_GROUND_SENSOR_YOFFSET, 
												  playerGroundCollisionMasks,
												  cleanBackground)))
		{
			Sound_Stop(SOUND_RUN);
			playerData->state = PLAYER_STATE_FALL;
		}
	}

	if (playerData->state == PLAYER_STATE_JUMP)
	{
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->jumpAirCounter--;
		playerData->safeLanding = TRUE;
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
		{
			playerData->speedy = PLAYER_MAX_FALL_SPEED;
			playerData->safeLanding = FALSE;
		}

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
			Sound_Play(SOUND_LAND, FALSE);
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;

			if (!playerData->safeLanding || playerData->isDead)
			{
				playerKill(playerData, framebuffer, cleanBackground);
				return;
			}
		}
	}
	else if (playerData->state == PLAYER_STATE_CLIMB)
	{
		playerData->speedy = 0;
		u8 testResult = testTerrainCollision(playerData->x, 
											 playerData->y, 
											 PLAYER_OFF_ROPE_SENSOR_YOFFSET, 
											 ropeCollisionMasks,
											 cleanBackground);

		u8 processLeftRight = TRUE;

		if (joystickState->jumpPressed)
		{
			// apply side movement if a direction was held
			if (joystickState->leftDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
				playerData->speedy = 0xff61;
				playerData->safeLanding = TRUE;
				playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
				playerData->state = PLAYER_STATE_JUMP;
				Sound_Play(SOUND_JUMP, FALSE);
				Sound_Stop(SOUND_CLIMB_UP);
				Sound_Stop(SOUND_CLIMB_DOWN);
				playerData->ignoreRopesCounter = 20;
			}
			else if (joystickState->rightDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
				playerData->facingDirection = PLAYER_FACING_RIGHT;
				playerData->speedy = 0xff61;
				playerData->safeLanding = TRUE;
				playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
				playerData->state = PLAYER_STATE_JUMP;
				Sound_Play(SOUND_JUMP, FALSE);
				Sound_Stop(SOUND_CLIMB_UP);
				Sound_Stop(SOUND_CLIMB_DOWN);
				playerData->ignoreRopesCounter = 20;
			}

			processLeftRight = FALSE;
		}
		else if (joystickState->upDown)
		{
			if (!playerData->speedy)
			{
				Sound_Play(SOUND_CLIMB_UP, TRUE);
			}

			playerData->speedy = PLAYER_CLIMB_UP_SPEED;

			playerData->currentFrameNumber = ((playerData->globalAnimationCounter >> 3) & 0x1) + 4;

			if (!TOUCHES_VINE(testResult))
			{
				playerData->speedy = 0;
				playerData->currentFrameNumber = PLAYER_CLIMB_FRAME_0;
				Sound_Stop(SOUND_CLIMB_UP);
			}

			processLeftRight = FALSE;
		}	
		else if (joystickState->downDown)
		{
			if (!playerData->speedy)
			{
				Sound_Play(SOUND_CLIMB_DOWN, TRUE);
			}

			playerData->speedy = PLAYER_CLIMB_DOWN_SPEED;
			playerData->currentFrameNumber = ((playerData->globalAnimationCounter >> 3) & 0x1) + 4;

			if (!TOUCHES_VINE(testResult))
			{
				playerData->state = PLAYER_STATE_FALL;
				Sound_Stop(SOUND_CLIMB_DOWN);
			}

			processLeftRight = FALSE;
		}
		else
		{
			Sound_Stop(SOUND_CLIMB_UP);
			Sound_Stop(SOUND_CLIMB_DOWN);
		}

		if (processLeftRight)
		{

			if (joystickState->leftDown)
			{
				Sound_Stop(SOUND_CLIMB_UP);
				Sound_Stop(SOUND_CLIMB_DOWN);

				playerData->holdLeftCounter++;
				if (playerData->holdLeftCounter > PLAYER_ROPE_HOLD_COUNT)
				{
					playerData->state = PLAYER_STATE_HANG_LEFT;
					playerData->x -= 4 << 8;
					playerData->holdLeftCounter = 0;
				}
			}
			else if (joystickState->rightDown)
			{
				Sound_Stop(SOUND_CLIMB_UP);
				Sound_Stop(SOUND_CLIMB_DOWN);

				playerData->holdRightCounter++;

				if (playerData->holdRightCounter > PLAYER_ROPE_HOLD_COUNT)
				{
					playerData->state = PLAYER_STATE_HANG_RIGHT;
					playerData->x += 4 << 8;
					playerData->holdRightCounter = 0;
				}
			}
			else
			{
				playerData->holdLeftCounter = 0;
				playerData->holdRightCounter = 0;
			}
		}
	}
	else if (playerData->state == PLAYER_STATE_HANG_LEFT)
	{
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->facingDirection = PLAYER_FACING_LEFT;

		if (joystickState->leftDown)
		{
			playerData->holdLeftCounter++;
			playerData->facingDirection = PLAYER_FACING_LEFT;

			if (playerData->holdLeftCounter > PLAYER_ROPE_HOLD_COUNT)
			{
				playerData->state = PLAYER_STATE_FALL;
				playerData->x -= 4 << 8;
				playerData->holdLeftCounter = 0;
			}
		}
		else if (joystickState->rightDown)
		{
			playerData->holdRightCounter++;
			playerData->facingDirection = PLAYER_FACING_RIGHT;

			if (playerData->holdRightCounter > PLAYER_ROPE_HOLD_COUNT)
			{
				playerData->state = PLAYER_STATE_CLIMB;
				playerData->currentFrameNumber = PLAYER_CLIMB_FRAME_0;
				playerData->x += 4 << 8;
				playerData->holdRightCounter = 0;
			}
		}
		else
		{
			playerData->holdLeftCounter = 0;
			playerData->holdRightCounter = 0;
		}
	}
	else if (playerData->state == PLAYER_STATE_HANG_RIGHT)
	{
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->facingDirection = PLAYER_FACING_RIGHT;

		if (joystickState->leftDown)
		{
			playerData->holdLeftCounter++;
			playerData->facingDirection = PLAYER_FACING_LEFT;

			if (playerData->holdLeftCounter > PLAYER_ROPE_HOLD_COUNT)
			{
				playerData->state = PLAYER_STATE_CLIMB;
				playerData->currentFrameNumber = PLAYER_CLIMB_FRAME_0;
				playerData->x -= 4 << 8;
				playerData->holdLeftCounter = 0;
			}
		}
		else if (joystickState->rightDown)
		{
			playerData->holdRightCounter++;
			playerData->facingDirection = PLAYER_FACING_RIGHT;
			if (playerData->holdRightCounter > PLAYER_ROPE_HOLD_COUNT)
			{
				playerData->state = PLAYER_STATE_FALL;
				playerData->x += 4 << 8;
				playerData->holdRightCounter = 0;
			}
		}
		else
		{
			playerData->holdLeftCounter = 0;
			playerData->holdRightCounter = 0;
		}

	}

	playerData->x += playerData->speedx;
	playerData->y += playerData->speedy;

	if (playerData->ignoreRopesCounter)
		playerData->ignoreRopesCounter--;

	//for (u8 loop = 0; loop < 12; loop++)
	//{
	//	debugSetPixel(((GET_HIGH_BYTE(playerData->x) << 1) + 6), GET_HIGH_BYTE(playerData->y) + loop, 0xffff0000);
	//	debugSetPixel(((GET_HIGH_BYTE(playerData->x) << 1) + 7), GET_HIGH_BYTE(playerData->y) + loop, 0xffff0000);
	//}


	//debugDrawBox(((GET_HIGH_BYTE(playerData->x) << 1) / 8) * 8, 
	//		     GET_HIGH_BYTE(playerData->y), 
	//		     16, 
	//		     16, 
	//		     0xff00ff00);

	/*
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
		!playerData->ignoreRopesCounter &&
		!playerData->isDead)
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
			playerData->safeLanding = TRUE;
			playerData->holdLeftCounter = 0;
			playerData->holdRightCounter = 0;
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

	if (playerData->state == PLAYER_STATE_CLIMB)
	{
		// erase the rope behind the player so that it 
		// doesn't blend with the player sprite, making it
		// appear on top
		for (u8 loop = 0; loop < 12; loop++)
		{
			// turn off the pixels on the rope
			u8 ropeX = GET_HIGH_BYTE(playerData->x) + 3;
			u16 location = GET_FRAMEBUFFER_LOCATION(ropeX, GET_HIGH_BYTE(playerData->y) + loop);
			framebuffer[location] &= ~pixelMasks[ropeX & 3];
		}

		//return;
	}

	if (playerData->state != PLAYER_STATE_REGENERATION)
	{
		drawSprite_24PixelsWide(playerData->currentSprite, 
								GET_HIGH_BYTE(playerData->x), 
								GET_HIGH_BYTE(playerData->y), 
								PLAYER_SPRITE_ROWS, 
								framebuffer);
	}
	else
	{
		drawSprite_24PixelsWide_static(playerData->currentSprite, 
									   GET_HIGH_BYTE(playerData->x), 
									   GET_HIGH_BYTE(playerData->y), 
									   PLAYER_SPRITE_ROWS, 
									   framebuffer);
	}

	// door touching check
	DoorInfo* doorInfoRunner = doorInfoData->doorInfos;
	for (u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
		if (GET_HIGH_BYTE(playerData->y) == doorInfoRunner->y &&
			GET_HIGH_BYTE(playerData->x) == doorInfoRunner->x)
		{
			if (doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask)
			{
				// jump to next room
				playerData->lastDoor = doorInfoRunner;
				return;
			}
		}

		doorInfoRunner++;
	}
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



BOOL objectCollisionTest(PlayerData* playerData, u8 x, u8 y, u8 width, u8 height)
{
	u8 playerX = GET_HIGH_BYTE(playerData->x);
	u8 playerY = GET_HIGH_BYTE(playerData->y);

	return (x < playerX + PLAYER_COLLISION_WIDTH &&
		    x + width > playerX &&
		    y < playerY + PLAYER_SPRITE_ROWS &&
		    y + height > playerY);
}

BOOL dropsManagerCollisionTest(DropData* dropData, PlayerData* playerData)
{
	const Drop* dropRunner = dropData->drops;

	for (u8 loop = 0; loop < dropData->activeDropsCount; loop++)
	{
		if ((s8)dropRunner->wiggleTimer > 0) // see note about wiggle time. 
											 // only test collision when it is positive in signed.
		{
			if (objectCollisionTest(playerData, 
									dropRunner->x, 
									GET_HIGH_BYTE(dropRunner->y),
									DROP_WIDTH,
									DROP_HEIGHT))
			{
				return TRUE;
			}

		}

		dropRunner++;
	}

	return FALSE;
}

void Player_PerformCollisions(struct GameData* gameDataStruct, 
							  Resources* resources)
{
	GameData* gameData = (GameData*) gameDataStruct;
	PlayerData* playerData = gameData->currentPlayerData;

	if (playerData->isDead)
		return;

	// collide with pickups
	u8 roomNumber = playerData->currentRoom->roomNumber;

	for (u8 loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		Pickup* pickUp = &playerData->gamePickups[roomNumber][loop];

		// is pickup active? Pickup state contain state for
		// both players.
		if (!(pickUp->state & playerData->playerMask))
			continue;

		if (objectCollisionTest(playerData, pickUp->x, pickUp->y, PICKUP_WIDTH, PICKUP_HEIGHT))
		{
			pickUp->state = pickUp->state & ~playerData->playerMask;

			Sound_Play(SOUND_PICKUP, FALSE);

			eraseSprite_16PixelsWide(resources->pickupSprites[pickUp->type],
									 pickUp->x, 
									 pickUp->y, 
									 PICKUPS_NUM_SPRITE_ROWS,
									 gameData->framebuffer,
									 gameData->cleanBackground);

			switch (pickUp->type)
			{
				case PICKUP_TYPE_KEY:
				{
					playerData->score += PICKUP_KEY_POINTS;

					// activate a door
					// draw a door if it's in the same room

					u8 doorIndex = pickUp->doorUnlockIndex;

					playerData->doorStateData[doorIndex] |= playerData->playerMask;

					// check if we need to activate a door in the room
					DoorInfoData* doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
					DoorInfo* doorInfoRunner = doorInfoData->doorInfos;

					for (u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
					{
						if (doorInfoRunner->globalDoorIndex == doorIndex) // the key actives a door in this room
						{
							if (doorInfoRunner->y != 0xffff) // initial invisible game door
							{
								drawDoor(doorInfoRunner, 
										resources->bitShiftedSprites_door, 
										gameData->framebuffer, 
										gameData->cleanBackground,
										TRUE /*draw on framebuffer*/);
							}
						}

						doorInfoRunner++;
					}

					break;
				}
				case PICKUP_TYPE_DIAMOND:
				{
					playerData->score += PICKUP_DIAMOND_POINTS;
					break;
				}
				case PICKUP_TYPE_MONEYBAG:
				{
					playerData->score += PICKUP_MONEYBAG_POINTS;
					break;
				}
			}

			// add a random value between 0 and 0x7f, as per the original game
			playerData->score += rand() % 0x7f;

			// update score and string
			convertScoreToString(playerData->score, playerData->scoreString);

			drawText(playerData->scoreString, 
					 resources->characterFont, 
					 gameData->framebuffer, 
					 SCORE_DRAW_LOCATION);
		}
	}

	if (playerData->state == PLAYER_STATE_DEBUG || 
		playerData->state == PLAYER_STATE_REGENERATION)
	{
		return;
	}

	// collide with drops
	if (dropsManagerCollisionTest(&gameData->dropData, playerData))
	{
		playerKill(playerData, 
				   gameData->framebuffer, 
				   gameData->cleanBackground);
		return;
	}

	// collide with ball
	BallData* ballData = &gameData->ballData;

	if (objectCollisionTest(playerData, 
							GET_HIGH_BYTE(ballData->x),
							GET_HIGH_BYTE(ballData->y),
							BALL_COLLISION_WIDTH,
							BALL_SPRITE_ROWS))
	{
		playerKill(playerData, 
				   gameData->framebuffer, 
				   gameData->cleanBackground);
		return;
	}

	// collide with bird
	BirdData* birdData = &gameData->birdData;

	if (objectCollisionTest(playerData, 
							GET_HIGH_BYTE(birdData->x),
							GET_HIGH_BYTE(birdData->y),
							BIRD_COLLISION_WIDTH,
							BIRD_SPRITE_ROWS))
	{
		playerKill(playerData, 
				   gameData->framebuffer, 
				   gameData->cleanBackground);
		return;
	}
}