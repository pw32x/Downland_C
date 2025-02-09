#include "game.h"

#include <string.h>

#include "draw_background.h"
#include "graphics_utils.h"

Resources* g_resources;

void Game_Init(GameData* gameData, Resources* resources)
{
	g_resources = resources;
}

void Game_Update(GameData* gameData)
{
	BackgroundDrawData* roomToDraw = &g_resources->backgroundDrawData_TitleScreen;

	Draw_Background(roomToDraw, 
					g_resources,
					gameData->framebuffer);

	Draw_Background(roomToDraw, 
					g_resources,
					gameData->cleanBackground);	

	drawText(g_resources->text_downland, 
			 g_resources->characterFont, 
			 gameData->framebuffer,
			 0x07c9);
}

void Game_Shutdown(GameData* gameData)
{

}
