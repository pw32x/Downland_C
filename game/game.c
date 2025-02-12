#include "game.h"

#include "rooms.h"

void Game_Init(GameData* gameData, Resources* resources)
{
	//gameData->gameCompletionCount = 0;
	gameData->resources = resources;
	gameData->numPlayers = 1;

	// init title screen
	Game_EnterRoom(gameData, TITLE_SCREEN_ROOM_INDEX);
}

void Game_Update(GameData* gameData)
{
	gameData->currentRoom->update((struct GameData*)gameData);
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
