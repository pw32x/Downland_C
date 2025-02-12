#include "game.h"

#include "rooms.h"

void Game_Init(GameData* gameData, Resources* resources)
{
	//gameData->gameCompletionCount = 0;
	gameData->resources = resources;

	// init title screen
	gameData->currentRoom = g_rooms[TITLE_SCREEN_ROOM_INDEX];
	gameData->currentRoom->init((struct Room*)gameData->currentRoom, 
								(struct GameData*)gameData, 
								resources);
}

void Game_Update(GameData* gameData)
{
	gameData->currentRoom->update((struct GameData*)gameData);
}

void Game_Shutdown(GameData* gameData)
{

}
