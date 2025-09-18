#include "chambers.h"
#include "room_types.h"

#include "../game.h"
#include "../game_data.h"
#include "../draw_utils.h"
#include "../drops_manager.h"
#include "../ball.h"
#include "../bird.h"
#include "../player.h"
#include "resources.h"

void updateTimers(dl_u8 roomNumber, dl_u16* roomTimers)
{
	int loop;

	for (loop = 0; loop < NUM_ROOMS; loop++)
	{
		if (loop == roomNumber)
		{
			if (*roomTimers)
				(*roomTimers)--;
		}
		else 
		{
			if (*roomTimers < ROOM_TIMER_DEFAULT)
				(*roomTimers)++;
		}

		roomTimers++;
	}
}

void chamber_draw(dl_u8 roomNumber);

void chamber_init(Room* room)
{
	dl_u8 roomNumber = room->roomNumber;
	PlayerData* playerData = gameData_currentPlayerData;

	// init drops
	DropsManager_Init(&res_roomResources[roomNumber].dropSpawnPositions,
					  roomNumber, 
					  playerData->gameCompletionCount);

	Ball_Init(roomNumber);
	Bird_Init();
	Player_RoomInit(playerData);

	convertScoreToString(playerData->score, playerData->scoreString);
	gameData_string_roomNumber[0] = roomNumber;
}

void chamber_update(Room* room)
{
	PlayerData* playerData = gameData_currentPlayerData;
	dl_u16 currentTimer;
	const DoorInfo* lastDoor;
	dl_u8 playerLives;
	PlayerData* temp;

	if (playerData->state != PLAYER_STATE_REGENERATION)
	{
		updateTimers(playerData->currentRoom->roomNumber, playerData->roomTimers);
	}

	currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

#ifndef DISABLE_ENEMIES
	DropsManager_Update(playerData->gameCompletionCount);	
#endif

	lastDoor = playerData->lastDoor;

	playerLives = playerData->lives;

	Player_Update(playerData, 
				  &res_roomResources[room->roomNumber].doorInfoData,
				  playerData->doorStateData);

	// player lost a life check
	if (playerData->lives < playerLives ||
		playerData->gameOver)
	{
		// if there's another player and they
		// have lives left, then switch.
		if (gameData_otherPlayerData != NULL &&
			!gameData_otherPlayerData->gameOver)
		{
			// switch players
			temp = gameData_otherPlayerData;
			gameData_otherPlayerData = gameData_currentPlayerData;
			gameData_currentPlayerData = temp;

			Game_TransitionToRoom(GET_READY_ROOM_INDEX);
			return;
		}
		else
		{
			// if there's only one player left and
			// they have lives left, then regen.
			// if not, then game over.

			if (playerData->gameOver)
			{
				Game_TransitionToRoom(TITLESCREEN_ROOM_INDEX);
				return;
			}
			else
			{
				if (birdData_state == BIRD_ACTIVE)
				{
					birdData_state = BIRD_SHUTDOWN;
				}

				if (!currentTimer)
				{
					currentTimer = ROOM_TIMER_HALF_TIME;
					playerData->roomTimers[playerData->currentRoom->roomNumber] = currentTimer;
				}

				Player_StartRegen(playerData);
			}
		}
	}

	if (lastDoor != playerData->lastDoor)
	{
		if (playerData->lastDoor->globalDoorIndex == LAST_DOOR_INDEX)
		{
			Player_CompleteGameLoop(playerData);
		}

		Game_WipeTransitionToRoom(playerData->lastDoor->nextRoomNumber);
		return;
	}

#ifndef DISABLE_ENEMIES
	Ball_Update();
	Bird_Update(currentTimer);
#endif

	// compute collisions
	// pick up item or die
	Player_PerformCollisions();

	convertTimerToString(currentTimer,
						 gameData_string_timer);
}

// All the chambers are the same, but this makes it easier
// to index them in g_rooms like any other screen.

Room chamber0 =
{
	0,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber1 =
{
	1,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber2 =
{
	2,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber3 =
{
	3,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber4 =
{
	4,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber5 =
{
	5,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber6 =
{
	6,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber7 =
{
	7,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber8 =
{
	8,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};

Room chamber9 =
{
	9,
	(InitRoomFunctionType)chamber_init,
	(DrawRoomFunctionType)chamber_draw,
	(UpdateRoomFunctionType)chamber_update
};
