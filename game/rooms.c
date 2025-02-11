#include "rooms.h"

#include <string.h>

#include "background_draw.h"
#include "game_types.h"
#include "graphics_utils.h"
#include "resource_types.h"

void titleScreen_init(GameData* gameData)
{
	Resources* resources = gameData->resources;

	gameData->roomNumber = 10;
	gameData->gameCompletionCount = 1; // act like the game was going through one for the title screen

	// init background and clean background

	gameData->dropData.dropSpawnPositions = &resources->drawSpawnPositions_room10;

	Background_Draw(&resources->backgroundDrawData_TitleScreen, 
					resources,
					gameData->framebuffer);

	// title screen text
	drawText(resources->text_downland, resources->characterFont, gameData->framebuffer, 0x03c9); // 0x07c9 original coco mem location
	drawText(resources->text_writtenBy, resources->characterFont, gameData->framebuffer, 0x050a); // 0x090A original coco mem location
	drawText(resources->text_michaelAichlmayer, resources->characterFont, gameData->framebuffer, 0x647); // 0x0A47 original coco mem location
	drawText(resources->text_copyright1983, resources->characterFont, gameData->framebuffer, 0x789); // 0x0B89 original coco mem location
	drawText(resources->text_spectralAssociates, resources->characterFont, gameData->framebuffer, 0x8c6); // 0x0CC6 original coco mem location
	drawText(resources->text_licensedTo, resources->characterFont, gameData->framebuffer, 0xa0a); // 0x0E0A original coco mem location
	drawText(resources->text_tandyCorporation, resources->characterFont, gameData->framebuffer, 0xb47); // 0x0F47 original coco mem location
	drawText(resources->text_allRightsReserved, resources->characterFont, gameData->framebuffer, 0xc86); // 0x1086 original coco mem location
	drawText(resources->text_onePlayer, resources->characterFont, gameData->framebuffer, 0xf05); // 0x1305 original coco mem location
	drawText(resources->text_twoPlayer, resources->characterFont, gameData->framebuffer, 0xf11); // 0x1311 original coco mem location
	drawText(resources->text_highScore, resources->characterFont, gameData->framebuffer, 0x118b); // 0x158B original coco mem location
	drawText(resources->text_playerOne, resources->characterFont, gameData->framebuffer, 0x1406); // 0x1806 original coco mem location
	drawText(resources->text_playerTwo, resources->characterFont, gameData->framebuffer, 0x1546); // 0x1946 original coco mem location

	memcpy(gameData->cleanBackground, gameData->framebuffer, FRAMEBUFFER_SIZE_IN_BYTES);
}

void titleScreen_update(GameData* gameData)
{

}

Room g_titleScreenRoom =
{
	NULL,
	(InitFunctionType)titleScreen_init,
	(UpdateFunctionType)titleScreen_update
};

Room* g_rooms[NUM_ROOMS] = 
{
	NULL,
	NULL,
	NULL
};