#include "game.h"

#include <memory.h>

#include "draw_utils.h"
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


void Game_Init(GameData* gameData, Resources* resources)
{
	//gameData->gameCompletionCount = 0;
	gameData->numPlayers = 1;
	gameData->currentPlayer = 1;
	gameData->playerLives = 3;

	// init strings
	gameData->string_roomNumber[ROOM_NUMBER_STRING_SIZE - 1] = 0xff; // end of line
	gameData->string_timer[TIMER_STRING_SIZE - 1] = 0xff;
	gameData->string_playerOneScore[SCORE_STRING_SIZE - 1] = 0xff;
	gameData->string_playerTwoScore[SCORE_STRING_SIZE - 1] = 0xff;
	gameData->string_highScore[SCORE_STRING_SIZE - 1] = 0xff;

	initPickups(gameData->gamePickups, 
				resources->roomPickupPositions,
				resources->keyPickUpDoorIndexes); // TODO check for hard mode after

	initDoors(gameData->doorStateData, 
			  resources->offsetsToDoorsAlreadyActivated);

	// init timers
	for (int loop = 0; loop < NUM_ROOMS; loop++)
		gameData->roomTimers[loop] = ROOM_TIMER_DEFAULT;

	gameData->playerOneScore = 0;
	gameData->playerTwoScore = 4433225;
	gameData->highScore =      9876543;

	gameData->playerData.playerMask = PLAYERONE_MASK;
	gameData->playerData.score = &gameData->playerOneScore;
	gameData->playerData.scoreString = gameData->string_playerOneScore;

	// init title screen
	//Game_EnterRoom(gameData, TITLESCREEN_ROOM_INDEX, resources);
	Game_TransitionToRoom(gameData, 0 /*TITLESCREEN_ROOM_INDEX*/, resources);
}

void Game_Update(GameData* gameData, Resources* resources)
{
	gameData->currentRoom->update((struct Room*)gameData->currentRoom, (struct GameData*)gameData, resources);
}

void Game_EnterRoom(GameData* gameData, u8 roomNumber, Resources* resources)
{
	gameData->currentRoom = g_rooms[roomNumber];
	gameData->currentRoom->init(gameData->currentRoom, 
								(struct GameData*)gameData, 
								resources);
}

void Game_TransitionToRoom(GameData* gameData, u8 roomNumber, Resources* resources)
{
	gameData->transitionRoomNumber = roomNumber;

	gameData->currentRoom = g_rooms[TRANSITION_ROOM_INDEX];
	gameData->currentRoom->init(g_rooms[roomNumber], 
								(struct GameData*)gameData, 
								resources);
}

void Game_Shutdown(GameData* gameData)
{

}
