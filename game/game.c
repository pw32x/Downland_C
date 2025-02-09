#include "game.h"

#include <string.h>

#include "draw_background.h"

Resources* g_resources;

void Game_Init(GameData* gameData, Resources* resources)
{
	memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE);
	g_resources = resources;
}

void Game_Update(GameData* gameData)
{
	Draw_Background(&g_resources->backgroundDrawData_room0_drawCommands, 
					g_resources,
					gameData->framebuffer);
}

void Game_Shutdown(GameData* gameData)
{

}
