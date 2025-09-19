#include "player.h"

#include "base_defines.h"
#include "physics_utils.h"
#include "game_data.h"
#include "pickup_types.h"
#include "dl_sound.h"
#include "dl_rand.h"
#include "dl_platform.h"
#include "bird.h"
#include "joystick_data.h"
#include "drops_manager.h"
#include "resources.h"

void playerKill(PlayerData* playerData);

dl_u8 playerX;
dl_u8 playerY;

dl_u8 object_x;
dl_u8 object_y;
dl_u8 object_width;
dl_u8 object_height;

BOOL objectCollisionTest(void)
{
	return !(object_x + object_width <= playerX ||
			 object_x >= playerX + PLAYER_COLLISION_WIDTH + 1 ||
			 object_y + object_height <= playerY ||
			 object_y >= playerY + PLAYER_COLLISION_HEIGHT);
}

#define setParams(x, y, width, height) \
object_x = x; \
object_y = y; \
object_width = width; \
object_height = height

BOOL dropsManagerCollisionTest(PlayerData* playerData)
{
	const Drop* dropRunner = dropData_drops;
	dl_u8 loop;

	for (loop = 0; loop < dropData_activeDropsCount; loop++)
	{
		if ((dl_s8)dropRunner->wiggleTimer > 0) // see note about wiggle time. 
											    // only test collision when it is positive in signed.
		{
			setParams(dropRunner->x, 
									GET_HIGH_BYTE(dropRunner->y),
									DROP_COLLISION_WIDTH,
									DROP_HEIGHT);

			if (objectCollisionTest())
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
	playerX = GET_HIGH_BYTE(playerData->x);
	playerY = GET_HIGH_BYTE(playerData->y) + 1;

	playerX += playerData->facingDirection ? 1 : 2;

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

		setParams(pickUp->x + 2, pickUp->y, PICKUP_WIDTH, PICKUP_HEIGHT);
		if (objectCollisionTest())
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
	setParams(GET_HIGH_BYTE(ballData_x) + 1,
							GET_HIGH_BYTE(ballData_y),
							BALL_COLLISION_WIDTH,
							BALL_SPRITE_ROWS);
	if (ballData_state == BALL_ACTIVE &&
		objectCollisionTest())
	{
		playerKill(playerData);
		return;
	}

	// collide with bird
	setParams(GET_HIGH_BYTE(birdData_x) + 1,
							GET_HIGH_BYTE(birdData_y),
							BIRD_COLLISION_WIDTH,
							BIRD_SPRITE_ROWS);
	if (birdData_state == BIRD_ACTIVE &&
		objectCollisionTest())
	{
		playerKill(playerData);
		return;
	}
}