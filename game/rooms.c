#include "rooms.h"

#include <string.h>

#include "background_draw.h"
#include "game_types.h"
#include "graphics_utils.h"
#include "drops_manager.h"
#include "game.h"

void titleScreen_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;
	gameData->gameCompletionCount = 1; // act like the game was going through one for the title screen

	// init background and clean background
	Background_Draw(&resources->roomResources[roomNumber].backgroundDrawData, 
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

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, gameData->gameCompletionCount);
}

void titleScreen_update(GameData* gameData)
{
	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						gameData->gameCompletionCount,
						gameData->resources->sprites_drops);

	if (gameData->joystickState.leftPressed)
	{
		gameData->numPlayers = 1;

	}
	else if (gameData->joystickState.rightPressed)
	{
		gameData->numPlayers = 2;
	}

	u16 drawLocation = gameData->numPlayers == 1 ? 0xf64 : 0xf70;  // hardcoded locations in the frambuffer
	u16 eraseLocation = gameData->numPlayers == 1 ? 0xf70 : 0xf64; // based on the original game.

	gameData->framebuffer[drawLocation] = 0xff;
	gameData->framebuffer[eraseLocation] = 0;

	if (gameData->joystickState.jumpPressed)
	{
		Game_TransitionToRoom(gameData, 0);
	}
}

Room titleScreenRoom =
{
	TITLE_SCREEN_ROOM_INDEX,
	(InitFunctionType)titleScreen_init,
	(UpdateFunctionType)titleScreen_update
};

void room_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;

	// init background and clean background
	Background_Draw(&resources->roomResources[roomNumber].backgroundDrawData, 
					resources,
					gameData->framebuffer);

	memcpy(gameData->cleanBackground, gameData->framebuffer, FRAMEBUFFER_SIZE_IN_BYTES);

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, gameData->gameCompletionCount);
}

void room_update(GameData* gameData)
{
	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						gameData->gameCompletionCount,
						gameData->resources->sprites_drops);	
}

Room room0 =
{
	0,
	(InitFunctionType)room_init,
	(UpdateFunctionType)room_update
};

void transition_init(Room* room, GameData* gameData, Resources* resources)
{
	// init clean background. we'll be slowly revealing it during the
	// room transition.
	Background_Draw(&resources->roomResources[gameData->transitionRoomNumber].backgroundDrawData, 
					resources,
					gameData->framebuffer);
}

void transition_update(GameData* gameData)
{
	Game_EnterRoom(gameData, gameData->transitionRoomNumber);
}

Room transitionRoom =
{
	TRANSITION_ROOM_INDEX,
	(InitFunctionType)transition_init,
	(UpdateFunctionType)transition_update
};

Room* g_rooms[NUM_ROOMS] = 
{
	&room0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&titleScreenRoom,
	&transitionRoom
};