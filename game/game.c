#include "game.h"

#include "drops_manager.h"
#include "rooms.h"

void Game_Init(GameData* gameData, Resources* resources)
{
	//gameData->gameCompletionCount = 0;
	gameData->resources = resources;

	// init title screen
	gameData->currentRoom = &g_titleScreenRoom;
	gameData->currentRoom->init((struct GameData*)gameData);

	// init drops
	DropsManager_Init(&gameData->dropData, gameData->roomNumber, gameData->gameCompletionCount);
}

void Game_Update(GameData* gameData)
{
	gameData->currentRoom->update((struct GameData*)gameData);

	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						gameData->gameCompletionCount,
						gameData->resources->sprites_drops);

}

void Game_Shutdown(GameData* gameData)
{

}
