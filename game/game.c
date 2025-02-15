#include "game.h"

#include <stdlib.h>

#include "rooms.h"

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

void Game_Init(GameData* gameData, Resources* resources)
{
	//gameData->gameCompletionCount = 0;
	gameData->resources = resources;
	gameData->numPlayers = 1;
	gameData->currentPlayer = 1;

	initPickups(gameData->gamePickups, 
				resources->roomPickupPositions,
				resources->keyPickUpDoorIndexes);

	// init title screen
	Game_EnterRoom(gameData, TITLE_SCREEN_ROOM_INDEX);
}

void Game_Update(GameData* gameData)
{
	gameData->currentRoom->update((struct Room*)gameData->currentRoom, (struct GameData*)gameData);
}

void Game_EnterRoom(GameData* gameData, u8 roomNumber)
{
	gameData->currentRoom = g_rooms[roomNumber];
	gameData->currentRoom->init(gameData->currentRoom, 
								(struct GameData*)gameData, 
								gameData->resources);
}

void Game_TransitionToRoom(GameData* gameData, u8 roomNumber)
{
	gameData->transitionRoomNumber = roomNumber;

	gameData->currentRoom = g_rooms[TRANSITION_ROOM_INDEX];
	gameData->currentRoom->init(g_rooms[roomNumber], 
								(struct GameData*)gameData, 
								gameData->resources);
}

void Game_Shutdown(GameData* gameData)
{

}
