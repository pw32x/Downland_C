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

	drawText(g_resources->text_downland, g_resources->characterFont, gameData->framebuffer, 0x03c9); // 0x07c9 original coco mem location
	drawText(g_resources->text_writtenBy, g_resources->characterFont, gameData->framebuffer, 0x050a); // 0x090A original coco mem location
	drawText(g_resources->text_michaelAichlmayer, g_resources->characterFont, gameData->framebuffer, 0x647); // 0x0A47 original coco mem location
	drawText(g_resources->text_copyright1983, g_resources->characterFont, gameData->framebuffer, 0x789); // 0x0B89 original coco mem location
	drawText(g_resources->text_spectralAssociates, g_resources->characterFont, gameData->framebuffer, 0x8c6); // 0x0CC6 original coco mem location
	drawText(g_resources->text_licensedTo, g_resources->characterFont, gameData->framebuffer, 0xa0a); // 0x0E0A original coco mem location
	drawText(g_resources->text_tandyCorporation, g_resources->characterFont, gameData->framebuffer, 0xb47); // 0x0F47 original coco mem location
	drawText(g_resources->text_allRightsReserved, g_resources->characterFont, gameData->framebuffer, 0xc86); // 0x1086 original coco mem location
	drawText(g_resources->text_onePlayer, g_resources->characterFont, gameData->framebuffer, 0xf05); // 0x1305 original coco mem location
	drawText(g_resources->text_twoPlayer, g_resources->characterFont, gameData->framebuffer, 0xf11); // 0x1311 original coco mem location
	drawText(g_resources->text_highScore, g_resources->characterFont, gameData->framebuffer, 0x118b); // 0x158B original coco mem location
	drawText(g_resources->text_playerOne, g_resources->characterFont, gameData->framebuffer, 0x1406); // 0x1806 original coco mem location
	drawText(g_resources->text_playerTwo, g_resources->characterFont, gameData->framebuffer, 0x1546); // 0x1946 original coco mem location



}

void Game_Shutdown(GameData* gameData)
{

}
