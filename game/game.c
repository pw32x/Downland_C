#include "game.h"

#include <string.h>

Resources* g_resources;

void Game_Init(GameData* gameData, Resources* resources)
{
	memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE);
	g_resources = resources;
}

void Game_Update(GameData* gameData)
{

}

void Game_Shutdown(GameData* gameData)
{

}
