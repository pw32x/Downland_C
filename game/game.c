#include "game.h"

#include "draw_utils.h"
#include "rooms\rooms.h"

void Game_Init(struct GameData* gameData, Resources* resources)
{
	gameData->targetFps = NORMAL_FPS;

	gameData->numPlayers = 1;

	// init strings
	gameData->string_roomNumber[ROOM_NUMBER_STRING_SIZE - 1] = 0xff; // end of line
	gameData->string_timer[TIMER_STRING_SIZE - 1] = 0xff;
	gameData->playerData[PLAYER_ONE].scoreString[SCORE_STRING_SIZE - 1] = 0xff;
	gameData->playerData[PLAYER_TWO].scoreString[SCORE_STRING_SIZE - 1] = 0xff;
	gameData->string_highScore[SCORE_STRING_SIZE - 1] = 0xff;

	gameData->highScore = 0;

	gameData->playerData[PLAYER_ONE].playerNumber = PLAYER_ONE;
	gameData->playerData[PLAYER_ONE].playerMask = PLAYERONE_BITMASK;

	gameData->playerData[PLAYER_TWO].playerNumber = PLAYER_TWO;
	gameData->playerData[PLAYER_TWO].playerMask = PLAYERTWO_BITMASK;

#ifdef START_AT_TITLESCREEN
	// init title screen
	Game_TransitionToRoom(gameData, TITLESCREEN_ROOM_INDEX, resources);
#else
	gameData->currentPlayerData = &gameData->playerData[PLAYER_ONE];
	gameData->otherPlayerData = NULL;
	Player_GameInit(gameData->currentPlayerData, resources);
	Game_TransitionToRoom(gameData, 0 /*roomNumber*/, resources);
#endif
}

void Game_Update(struct GameData* gameData, Resources* resources)
{
	gameData->currentRoom->update((struct Room*)gameData->currentRoom, 
								  (struct GameData*)gameData, 
								  resources);
}

void Game_EnterRoom(struct GameData* gameData, u8 roomNumber, Resources* resources)
{
	gameData->currentRoom = g_rooms[roomNumber];

	if (gameData->currentPlayerData != NULL)
		gameData->currentPlayerData->currentRoom = gameData->currentRoom;

	gameData->currentRoom->init(gameData->currentRoom, 
								(struct GameData*)gameData, 
								resources);
}

void Game_TransitionToRoom(struct GameData* gameData, u8 roomNumber, Resources* resources)
{
	gameData->transitionRoomNumber = roomNumber;

	gameData->currentRoom = g_rooms[roomNumber];

	if (gameData->currentPlayerData != NULL)
		gameData->currentPlayerData->currentRoom = gameData->currentRoom;

	gameData->currentRoom = g_rooms[TRANSITION_ROOM_INDEX];
	gameData->currentRoom->init(g_rooms[roomNumber], 
								(struct GameData*)gameData, 
								resources);
}

void Game_WipeTransitionToRoom(struct GameData* gameData, u8 roomNumber, Resources* resources)
{
	gameData->transitionRoomNumber = roomNumber;

	gameData->currentRoom = g_rooms[roomNumber];

	if (gameData->currentPlayerData != NULL)
		gameData->currentPlayerData->currentRoom = gameData->currentRoom;

	gameData->currentRoom = g_rooms[WIPE_TRANSITION_ROOM_INDEX];
	gameData->currentRoom->init(g_rooms[roomNumber], 
								(struct GameData*)gameData, 
								resources);
}

void Game_Shutdown(struct GameData* gameData)
{

}
