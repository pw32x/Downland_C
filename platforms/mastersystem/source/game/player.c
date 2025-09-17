#include "player.h"

#include "base_defines.h"
#include "draw_utils.h"
#include "physics_utils.h"
#include "game_data.h"
#include "pickup_types.h"
#include "dl_sound.h"
#include "dl_rand.h"
#include "dl_platform.h"
#include "bird.h"
#include "joystick_data.h"
#include "drops_manager.h"

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

void playerKill(PlayerData* playerData)
{
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

	dl_memset(doorStateData, 0, DOOR_TOTAL_COUNT);

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

	// init timers
	for (loop = 0; loop < NUM_ROOMS; loop++)
		playerData->roomTimers[loop] = ROOM_TIMER_DEFAULT;
}

void Player_RoomInit(PlayerData* playerData)
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
}

void Player_StartRegen(PlayerData* playerData)
{
	playerData->state = PLAYER_STATE_REGENERATION;
	playerData->currentFrameNumber = PLAYER_RUN_FRAME_0_STAND;
	playerData->regenerationCounter = PLAYER_REGENERATION_TIME;
	playerData->cantMoveCounter = PLAYER_REGENERATION_IMMOBILE_TIME;
}

void Player_Update(PlayerData* playerData, 
				   const DoorInfoData* doorInfoData,
				   dl_u8* doorStateData)
{
	const DoorInfo* doorInfoRunner;
	dl_u8 testResult;
	dl_u8 processLeftRight;
	dl_u8 loop;

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
	if (joystickState_debugStatePressed)
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

	if (playerData->state == PLAYER_STATE_DEBUG)
	{
		// apply side movement if a direction was held
		if (joystickState_leftDown)
		{
			playerData->x -= PLAYER_DEBUG_SPEED;
		}
		else if (joystickState_rightDown)
		{
			playerData->x += PLAYER_DEBUG_SPEED;
		}

		if (joystickState_upDown)
		{
			playerData->y -= PLAYER_DEBUG_SPEED;
		}	
		else if (joystickState_downDown)
		{
			playerData->y += PLAYER_DEBUG_SPEED;
		}
	}

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		if (joystickState_leftDown ||
			joystickState_rightDown ||
			joystickState_upDown ||
			joystickState_downDown)
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

		if (joystickState_jumpPressed)
		{
			playerData->speedy = 0xff61;
			playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
			playerData->state = PLAYER_STATE_JUMP;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
			Sound_Play(SOUND_JUMP, FALSE);

			// apply side movement if a direction was held
			if (joystickState_leftDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
			}
			else if (joystickState_rightDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
				playerData->facingDirection = PLAYER_FACING_RIGHT;
			}
		}
		else if (joystickState_leftDown ||
				 joystickState_rightDown)
		{
			playerData->state = PLAYER_STATE_RUN;
		}		
	}

	else if (playerData->state == PLAYER_STATE_RUN)
	{
		if (joystickState_jumpPressed)
		{
			playerData->speedy = 0xff61;
			playerData->jumpAirCounter = PLAYER_JUMP_AIR_COUNT;
			playerData->state = PLAYER_STATE_JUMP;
			playerData->currentFrameNumber = PLAYER_RUN_FRAME_2_JUMP;
			Sound_Stop(SOUND_RUN);
			Sound_Play(SOUND_JUMP, FALSE);

			if (joystickState_leftDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
			}
			else if (joystickState_rightDown)
			{
				playerData->speedx = PLAYER_RUN_SPEED_RIGHT;
				playerData->facingDirection = PLAYER_FACING_RIGHT;
			}
		}
		else
		{
			if (joystickState_leftDown)
			{
				if (!playerData->speedx)
				{
					Sound_Play(SOUND_RUN, TRUE);
				}

				playerData->speedx = PLAYER_RUN_SPEED_LEFT;
				playerData->facingDirection = PLAYER_FACING_LEFT;
			}
			else if (joystickState_rightDown)
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
													  playerGroundCollisionMasks)))
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
												 playerGroundCollisionMasks)))
		{
			dl_u8 killPlayer = (playerData->speedy == PLAYER_MAX_FALL_SPEED || playerData->isDead);

			playerData->state = PLAYER_STATE_STAND;
			playerData->preserveAirMomentum = FALSE;
			playerData->speedy = 0;

			if (killPlayer)
			{
				playerKill(playerData);
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
										  ropeCollisionMasks);

		processLeftRight = TRUE;

		if (joystickState_jumpPressed)
		{
			// apply side movement if a direction was held
			if (joystickState_leftDown)
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
			else if (joystickState_rightDown)
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
		else if (joystickState_upDown)
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
		else if (joystickState_downDown)
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

			if (joystickState_leftDown)
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
			else if (joystickState_rightDown)
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

		if (joystickState_leftDown)
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
		else if (joystickState_rightDown)
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

		if (joystickState_leftDown)
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
		else if (joystickState_rightDown)
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
											 ropeCollisionMasks);
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
						playerGroundCollisionMasks)))
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

BOOL objectCollisionTest(PlayerData* playerData, dl_u8 x, dl_u8 y, dl_u8 width, dl_u8 height)
{
	dl_u8 playerX = GET_HIGH_BYTE(playerData->x);
	dl_u8 playerY = GET_HIGH_BYTE(playerData->y) + 1;

	playerX += playerData->facingDirection ? 1 : 2;

	return (x < playerX + PLAYER_COLLISION_WIDTH + 1 &&
		    x + width > playerX &&
		    y < playerY + PLAYER_COLLISION_HEIGHT &&
		    y + height > playerY);
}

BOOL dropsManagerCollisionTest(PlayerData* playerData)
{
	const Drop* dropRunner = dropData_drops;
	dl_u8 loop;

	for (loop = 0; loop < dropData_activeDropsCount; loop++)
	{
		if ((dl_s8)dropRunner->wiggleTimer > 0) // see note about wiggle time. 
											 // only test collision when it is positive in signed.
		{
			if (objectCollisionTest(playerData, 
									dropRunner->x, 
									GET_HIGH_BYTE(dropRunner->y),
									DROP_COLLISION_WIDTH,
									DROP_HEIGHT))
			{
				return TRUE;
			}

		}

		dropRunner++;
	}

	return FALSE;
}

void Player_PerformCollisions(void)
{
	dl_u8 roomNumber;
	dl_u8 loop;
	Pickup* pickUp;
	dl_u8 doorIndex;

	PlayerData* playerData = gameData_currentPlayerData;

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

		if (objectCollisionTest(playerData, pickUp->x + 2, pickUp->y, PICKUP_WIDTH, PICKUP_HEIGHT))
		{
			pickUp->state = pickUp->state & ~playerData->playerMask;

			Sound_Play(SOUND_PICKUP, FALSE);

			switch (pickUp->type)
			{
				case PICKUP_TYPE_KEY:
				{
					playerData->score += PICKUP_KEY_POINTS;

					// activate a door
					// draw a door if it's in the same room

					doorIndex = pickUp->doorUnlockIndex;

					playerData->doorStateData[doorIndex] |= playerData->playerMask;

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
		}
	}

	if (playerData->state == PLAYER_STATE_DEBUG || 
		playerData->state == PLAYER_STATE_REGENERATION)
	{
		return;
	}

	// collide with drops
	if (dropsManagerCollisionTest(playerData))
	{
		playerKill(playerData);
		return;
	}

	// collide with ball
	if (ballData_state == BALL_ACTIVE &&
		objectCollisionTest(playerData, 
							GET_HIGH_BYTE(ballData_x) + 1,
							GET_HIGH_BYTE(ballData_y),
							BALL_COLLISION_WIDTH,
							BALL_SPRITE_ROWS))
	{
		playerKill(playerData);
		return;
	}

	// collide with bird
	if (birdData_state == BIRD_ACTIVE &&
		objectCollisionTest(playerData, 
							GET_HIGH_BYTE(birdData_x) + 1,
							GET_HIGH_BYTE(birdData_y),
							BIRD_COLLISION_WIDTH,
							BIRD_SPRITE_ROWS))
	{
		playerKill(playerData);
		return;
	}
}