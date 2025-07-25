#include "player.h"

#include <string.h>

#include "base_defines.h"
#include "draw_utils.h"
#include "physics_utils.h"
#include "debug_utils.h"
#include "game_types.h"
#include "pickup_types.h"
#include "door_utils.h"
#include "dl_sound.h"
#include "dl_rand.h"



#define PLAYER_START_LIVES 3

#define PLAYER_REGENERATION_TIME			0x190 // 400
#define PLAYER_REGENERATION_IMMOBILE_TIME	0x28  // 40
#define PLAYER_SPLAT_INITIAL_FREEZE_TIME	0xa	  // 10
#define PLAYER_SPLAT_ANIMATION_TRIGGER_TIME 0x46  // 70
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

#define PLAYER_WALL_SENSOR_YOFFSET		15
#define PLAYER_GROUND_SENSOR_YOFFSET	16
#define PLAYER_ROPE_SENSOR_YOFFSET		8
#define PLAYER_OFF_ROPE_SENSOR_YOFFSET	7

// individual sprites
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

// used for animation
// - run is always 0 to 3, no matter what direction
// the player is facing. the correct sprite is
// chosen right before drawing.
// - climb alternates between frames 4 and 5, which
// match the sprites above.
// - standing is run frame 0
// - jumping is run frame 2
#define PLAYER_RUN_FRAME_0_STAND		0
#define PLAYER_RUN_FRAME_1				1
#define PLAYER_RUN_FRAME_2_JUMP			2
#define PLAYER_RUN_FRAME_3				3
#define PLAYER_CLIMB_FRAME_0			4
#define PLAYER_CLIMB_FRAME_1			5
#define PLAYER_FRAME_COUNT				6

// used to determine whether the player
// is touching the ground
dl_u16 playerGroundCollisionMasks[4] =
{
	0x03c0, // 0000001111000000b
    0x00f0, // 0000000011110000b
    0x003c, // 0000000000111100b
    0x0f00, // 0000111100000000b
};

// used to determine whether the player
// is touching a rope.
dl_u16 ropeCollisionMasks[4] = 
{
    0x0300, // 0000001100000000b
    0x00c0, // 0000000011000000b
    0x0030, // 0000000000110000b
    0x0c00, // 0000110000000000b
};

void playerKill(PlayerData* playerData, dl_u8* framebuffer, dl_u8* cleanBackground)
{
	dl_u8 x;
	dl_u8 y;
	const dl_u8* splatSprite;

	playerData->speedx = 0;
	playerData->speedy = 0;
	playerData->regenerationCounter = 0;
	playerData->isDead = TRUE;

	Sound_Stop(SOUND_RUN);
	Sound_Stop(SOUND_CLIMB_UP);
	Sound_Stop(SOUND_CLIMB_DOWN);

	if (playerData->state == PLAYER_STATE_STAND ||
		playerData->state == PLAYER_STATE_RUN)
	{
		playerData->state = PLAYER_STATE_SPLAT;
		Sound_Play(SOUND_SPLAT, FALSE);
		playerData->cantMoveCounter = PLAYER_SPLAT_WAIT_TIME;

		x = GET_HIGH_BYTE(playerData->x);
		y = GET_HIGH_BYTE(playerData->y);

		// erase the player, then draw the splat sprite
		// we won't redraw this again, like the original game
		eraseSprite_24PixelsWide_simple(x,
										y,
										PLAYER_SPRITE_ROWS,
										framebuffer, 
										cleanBackground);

		splatSprite = getBitShiftedSprite(playerData->bitShiftedSplatSprite, 
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
		playerData->preserveAirMomentum = FALSE;
	}
}

dl_u8 computeSpriteNumber(dl_u8 facingDirection, dl_u8 currentFrameNumber)
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
				 const dl_u8* keyPickUpDoorIndexes)
{
	int loop;
	int innerLoop;

	// init all the keys for all the rooms for both players

	dl_u8 pickUpTypes[NUM_PICKUPS_PER_ROOM];
	pickUpTypes[0] = PICKUP_TYPE_KEY; // the first two pickups are always keys
	pickUpTypes[1] = PICKUP_TYPE_KEY;

	for (loop = 0; loop < NUM_ROOMS; loop++)
	{
		Pickup* pickups = roomPickups[loop];

		// the last three pickups are random between diamonds and money bags
		pickUpTypes[2] = dl_rand() % 2;
		pickUpTypes[3] = dl_rand() % 2;
		pickUpTypes[4] = dl_rand() % 2;

		for (innerLoop = 0; innerLoop < NUM_PICKUPS_PER_ROOM; innerLoop++)
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

void initDoors(dl_u8* doorStateData, const dl_u8* offsetsToDoorsAlreadyActivated)
{
	dl_u8 alreadyOpenedState = 0x3; // set the two bits for each player

	memset(doorStateData, 0, DOOR_TOTAL_COUNT);

	while (*offsetsToDoorsAlreadyActivated != 0xff)
	{
		doorStateData[*offsetsToDoorsAlreadyActivated] = alreadyOpenedState;

		offsetsToDoorsAlreadyActivated++;
	}
}

void playerStartGameLoop(PlayerData* playerData, const Resources* resources)
{
	const dl_u8* keyPickUpDoorIndexes = playerData->gameCompletionCount == 0 ? resources->keyPickUpDoorIndexes : resources->keyPickUpDoorIndexesHardMode;

	playerData->facingDirection = PLAYER_FACING_LEFT;

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
	int loop;

	playerStartGameLoop(playerData, resources);

	playerData->lastDoor = NULL;
	playerData->lives = PLAYER_START_LIVES;
	playerData->isDead = 0;
	playerData->gameOver = FALSE;
	playerData->score = 0;
	playerData->gameCompletionCount = 0;

	playerData->bitShiftedSprites = resources->bitShiftedSprites_player;
	playerData->bitShiftedCollisionMasks = resources->bitShiftedCollisionmasks_player;
	playerData->bitShiftedSplatSprite = resources->bitShiftedSprites_playerSplat;

	// init timers
	for (loop = 0; loop < NUM_ROOMS; loop++)
		playerData->roomTimers[loop] = ROOM_TIMER_DEFAULT;
}

void Player_RoomInit(PlayerData* playerData, const Resources* resources)
{
	// setup initial state for room

	if (playerData->isDead)
	{
		// we've died in a room and need to regenerate
		Player_StartRegen(playerData);
		playerData->isDead = FALSE;
	}
	else if (playerData->lastDoor)
	{	
		// we've entered a room
		if (playerData->state != PLAYER_STATE_DEBUG)
			playerData->state = PLAYER_STATE_STAND;

		// we've gone through a door
		playerData->x = SET_HIGH_BYTE(playerData->lastDoor->xLocationInNextRoom);
		playerData->y = SET_HIGH_BYTE(playerData->lastDoor->yLocationInNextRoom);
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
	}
	else
	{
		// we're starting the game
		playerData->state = PLAYER_STATE_STAND;
		Player_StartRegen(playerData);
		playerData->x = SET_HIGH_BYTE(PLAYER_START_X);
		playerData->y = SET_HIGH_BYTE(PLAYER_START_Y);
	}

	playerData->splatFrameNumber = 0;

	playerData->speedx = 0;
	playerData->speedy = 0;

	playerData->preserveAirMomentum = FALSE;
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
				   dl_u8* framebuffer, 
				   dl_u8* cleanBackground,
				   const DoorInfoData* doorInfoData,
				   dl_u8* doorStateData)
{
	dl_u8 x;
	dl_u8 y;
	dl_u8 testResult;
	dl_u8 processLeftRight;
	dl_u8 loop;
	dl_u8 ropeX;
	dl_u16 location;
	const DoorInfo* doorInfoRunner;

	playerData->globalAnimationCounter++;

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
			playerData->splatFrameNumber = 1;

			x = GET_HIGH_BYTE(playerData->x);
			y = GET_HIGH_BYTE(playerData->y);

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
		Sound_Stop(SOUND_RUN);
		Sound_Stop(SOUND_CLIMB_UP);
		Sound_Stop(SOUND_CLIMB_DOWN);

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
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
			playerData->speedy = 0;
			playerData->preserveAirMomentum = TRUE;
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
			playerData->splatFrameNumber = 0;
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
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
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

	else if (playerData->state == PLAYER_STATE_RUN)
	{
		if (joystickState->jumpPressed)
		{
			playerData->speedy = 0xff61;
			playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
			playerData->state = PLAYER_STATE_JUMP;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
			Sound_Stop(SOUND_RUN);
			Sound_Play(SOUND_JUMP, FALSE);

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
		else
		{
			if (joystickState->leftDown)
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
	}

	else if (playerData->state == PLAYER_STATE_JUMP)
	{
		playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
		playerData->jumpAirCounter--;
		playerData->preserveAirMomentum = TRUE;
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
			playerData->preserveAirMomentum = FALSE;
		}

		if (!playerData->preserveAirMomentum)
		{
			// when past the max fall speed, reduce the x speed
			// like the player is losing momentum.
			if (playerData->facingDirection == PLAYER_FACING_LEFT)
			{
				if (playerData->speedx != 0)
					playerData->speedx++;
			}
			else
			{
				if (playerData->speedx > 0)
					playerData->speedx--;
			}
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
			dl_u8 killPlayer = (playerData->speedy == PLAYER_MAX_FALL_SPEED || playerData->isDead);

			playerData->state = PLAYER_STATE_STAND;
			playerData->preserveAirMomentum = FALSE;
			playerData->speedy = 0;

			if (killPlayer)
			{
				playerKill(playerData, framebuffer, cleanBackground);
				return;
			}

			Sound_Play(SOUND_LAND, FALSE);
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
		}
	}
	else if (playerData->state == PLAYER_STATE_CLIMB)
	{
		playerData->speedy = 0;
		testResult = testTerrainCollision(playerData->x, 
										  playerData->y, 
										  PLAYER_OFF_ROPE_SENSOR_YOFFSET, 
										  ropeCollisionMasks,
										  cleanBackground);

		processLeftRight = TRUE;

		if (joystickState->jumpPressed)
		{
			// apply side movement if a direction was held
			if (joystickState->leftDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
				playerData->speedy = 0xff61;
				playerData->preserveAirMomentum = FALSE;
				playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
				playerData->state = PLAYER_STATE_JUMP;
				playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
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
				playerData->preserveAirMomentum = FALSE;
				playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
				playerData->state = PLAYER_STATE_JUMP;
				playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
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

	// if in the air, check for ropes
	if ((playerData->state == PLAYER_STATE_JUMP ||
		playerData->state == PLAYER_STATE_FALL) &&
		!playerData->ignoreRopesCounter &&
		!playerData->isDead)
	{
		dl_u8 testResult = testTerrainCollision(playerData->x, 
											 playerData->y, 
											 PLAYER_ROPE_SENSOR_YOFFSET, 
											 ropeCollisionMasks,
											 cleanBackground);
		if (TOUCHES_VINE(testResult))
		{
			Sound_Stop(SOUND_JUMP);
			playerData->state = PLAYER_STATE_CLIMB;
			playerData->speedx = 0;
			playerData->speedy = 0;
			playerData->preserveAirMomentum = FALSE;
			playerData->holdLeftCounter = 0;
			playerData->holdRightCounter = 0;
			playerData->currentFrameNumber = PLAYER_CLIMB_FRAME_0;
			playerData->jumpAirCounter = 0;
		}
	}

	// wall detection when moving
	if (TOUCHES_TERRAIN(testTerrainCollision(playerData->x, 
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
			Sound_Stop(SOUND_RUN);
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
		}
		else if (playerData->state == PLAYER_STATE_JUMP)
		{
			Sound_Stop(SOUND_JUMP);
			playerData->speedx = -playerData->speedx;
			playerData->jumpAirCounter = 1;
			playerData->facingDirection = !playerData->facingDirection;
		}
		else if (playerData->state == PLAYER_STATE_FALL)
		{
			Sound_Stop(SOUND_JUMP);
		}
	}

	// from the frame number, get the actual sprite to use
	playerData->currentSpriteNumber = computeSpriteNumber(playerData->facingDirection, playerData->currentFrameNumber);

	// get the sprite for the current horizontal bit the player is on
	playerData->currentSprite = getBitShiftedSprite(playerData->bitShiftedSprites, 
												    playerData->currentSpriteNumber,
												    GET_HIGH_BYTE(playerData->x) & 3, 
												    PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE);

	if (playerData->state == PLAYER_STATE_CLIMB)
	{
		// erase the rope behind the player so that it 
		// doesn't blend with the player sprite, making it
		// appear on top
		for (loop = 0; loop < 12; loop++)
		{
			// turn off the pixels on the rope
			ropeX = GET_HIGH_BYTE(playerData->x) + 3;
			location = GET_FRAMEBUFFER_LOCATION(ropeX, GET_HIGH_BYTE(playerData->y) + loop);
			framebuffer[location] &= ~pixelMasks[ropeX & 3];
		}
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
	doorInfoRunner = doorInfoData->doorInfos;
	for (loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
		if (GET_HIGH_BYTE(playerData->y) == doorInfoRunner->y &&
			GET_HIGH_BYTE(playerData->x) == doorInfoRunner->x)
		{
			if (doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask)
			{
				// jump to next room
				Sound_Stop(SOUND_RUN);
				Sound_Stop(SOUND_JUMP);
				playerData->lastDoor = doorInfoRunner;
				return;
			}
		}

		doorInfoRunner++;
	}
}

dl_u8 collisionTestBuffer[PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE];
dl_u8 utilityBuffer[PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE];

dl_u8 Player_HasCollision(PlayerData* playerData, dl_u8* framebuffer, dl_u8* cleanBackground)
{
	dl_u8* cleanBackgroundRunner;
	dl_u8* collisionTestBufferRunner;
	dl_u8 loop;
	const dl_u8* currentSprite;

	dl_u8 sensorX = GET_HIGH_BYTE(playerData->x);

	const dl_u8* currentCollisionMask = getBitShiftedSprite(playerData->bitShiftedCollisionMasks, 
														 playerData->currentSpriteNumber,
														 sensorX & 3, 
														 PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE);

	dl_u8 sensorY = GET_HIGH_BYTE(playerData->y) + 1; // start one line down
	dl_u16 location = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

	framebuffer = &framebuffer[location];
	cleanBackgroundRunner = &cleanBackground[location];

	collisionTestBufferRunner = collisionTestBuffer;


	for (loop = 0; loop < PLAYER_COLLISION_MASK_ROWS; loop++)
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

	currentSprite = playerData->currentSprite + 3; // start one line down
	cleanBackgroundRunner = &cleanBackground[location];
	collisionTestBufferRunner = collisionTestBuffer;

	for (loop = 0; loop < PLAYER_COLLISION_MASK_ROWS; loop++)
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



BOOL objectCollisionTest(PlayerData* playerData, dl_u8 x, dl_u8 y, dl_u8 width, dl_u8 height)
{
	dl_u8 playerX = GET_HIGH_BYTE(playerData->x);
	dl_u8 playerY = GET_HIGH_BYTE(playerData->y);

	return (x < playerX + PLAYER_COLLISION_WIDTH &&
		    x + width > playerX &&
		    y < playerY + PLAYER_SPRITE_ROWS &&
		    y + height > playerY);
}

BOOL dropsManagerCollisionTest(DropData* dropData, PlayerData* playerData)
{
	const Drop* dropRunner = dropData->drops;
	dl_u8 loop;

	for (loop = 0; loop < dropData->activeDropsCount; loop++)
	{
		if ((dl_s8)dropRunner->wiggleTimer > 0) // see note about wiggle time. 
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
							  const Resources* resources)
{
	dl_u8 roomNumber;
	dl_u8 loop;
	Pickup* pickUp;
#ifndef DISABLE_DOOR_DRAWING
	dl_u8 doorLoop;
	const DoorInfoData* doorInfoData;
	const DoorInfo* doorInfoRunner;
#endif
	BallData* ballData;
	BirdData* birdData;
	dl_u8 doorIndex;

	GameData* gameData = (GameData*) gameDataStruct;
	PlayerData* playerData = gameData->currentPlayerData;

	if (playerData->isDead)
		return;

	// collide with pickups
	roomNumber = playerData->currentRoom->roomNumber;

	for (loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		pickUp = &playerData->gamePickups[roomNumber][loop];

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

					doorIndex = pickUp->doorUnlockIndex;

					playerData->doorStateData[doorIndex] |= playerData->playerMask;

#ifndef DISABLE_DOOR_DRAWING
					// check if we need to activate a door in the room
					doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
					doorInfoRunner = doorInfoData->doorInfos;

					for (doorLoop = 0; doorLoop < doorInfoData->drawInfosCount; doorLoop++)
					{
						if (doorInfoRunner->globalDoorIndex == doorIndex) // the key actives a door in this room
						{
							if (doorInfoRunner->x != 0xff &&
								doorInfoRunner->y != 0xff ) // initial invisible game door
							{
								drawDoor(doorInfoRunner, 
										resources->bitShiftedSprites_door, 
										gameData->framebuffer, 
										gameData->cleanBackground,
										TRUE ); //draw on framebuffer
							}
						}

						doorInfoRunner++;
					}
#endif
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
			playerData->score += dl_rand() % 0x7f;

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
	ballData = &gameData->ballData;

	if (ballData->state == BALL_ACTIVE &&
		objectCollisionTest(playerData, 
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
	birdData = &gameData->birdData;

	if (birdData->state == BIRD_ACTIVE &&
		objectCollisionTest(playerData, 
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