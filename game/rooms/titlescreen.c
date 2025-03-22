#include "room_types.h"

#include <string.h>

#include "..\game_types.h"
#include "..\draw_utils.h"
#include "..\drops_manager.h"
#include "..\sound.h"

#define TITLESCREEN_HIGHSCORE_LOCATION			0x12cc
#define TITLESCREEN_PLAYERONE_SCORE_LOCATION	0x1412
#define TITLESCREEN_PLAYERTWO_SCORE_LOCATION	0x1552

void titleScreen_draw(u8 roomNumber, GameData* gameData, Resources* resources)
{
	u8* framebuffer = gameData->cleanBackground;

	// init background and text
	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
				   resources,
				   framebuffer);

	// title screen text
	drawText(resources->text_downland, resources->characterFont, framebuffer, 0x03c9); // 0x07c9 original coco mem location
	drawText(resources->text_writtenBy, resources->characterFont, framebuffer, 0x050a); // 0x090A original coco mem location
	drawText(resources->text_michaelAichlmayer, resources->characterFont, framebuffer, 0x647); // 0x0A47 original coco mem location
	drawText(resources->text_copyright1983, resources->characterFont, framebuffer, 0x789); // 0x0B89 original coco mem location
	drawText(resources->text_spectralAssociates, resources->characterFont, framebuffer, 0x8c6); // 0x0CC6 original coco mem location
	drawText(resources->text_licensedTo, resources->characterFont, framebuffer, 0xa0a); // 0x0E0A original coco mem location
	drawText(resources->text_tandyCorporation, resources->characterFont, framebuffer, 0xb47); // 0x0F47 original coco mem location
	drawText(resources->text_allRightsReserved, resources->characterFont, framebuffer, 0xc86); // 0x1086 original coco mem location
	drawText(resources->text_onePlayer, resources->characterFont, framebuffer, 0xf05); // 0x1305 original coco mem location
	drawText(resources->text_twoPlayer, resources->characterFont, framebuffer, 0xf11); // 0x1311 original coco mem location
	drawText(resources->text_highScore, resources->characterFont, framebuffer, 0x118b); // 0x158B original coco mem location
	drawText(resources->text_playerOne, resources->characterFont, framebuffer, 0x1406); // 0x1806 original coco mem location
	drawText(resources->text_playerTwo, resources->characterFont, framebuffer, 0x1546); // 0x1946 original coco mem location

	convertScoreToString(gameData->playerData[PLAYER_ONE].score, gameData->playerData[PLAYER_ONE].scoreString);

	drawText(gameData->playerData[PLAYER_ONE].scoreString, 
			 resources->characterFont, 
			 framebuffer, 
			 TITLESCREEN_PLAYERONE_SCORE_LOCATION);

	convertScoreToString(gameData->playerData[PLAYER_TWO].score, gameData->playerData[PLAYER_TWO].scoreString);

	drawText(gameData->playerData[PLAYER_TWO].scoreString, 
			 resources->characterFont, 
			 framebuffer, 
			 TITLESCREEN_PLAYERTWO_SCORE_LOCATION);

	convertScoreToString(gameData->highScore, gameData->string_highScore);

	drawText(gameData->string_highScore, 
			 resources->characterFont, 
			 framebuffer, 
			 TITLESCREEN_HIGHSCORE_LOCATION);
}

void titleScreen_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, 1 /*gameCompletionCount*/);
}

void titleScreen_update(Room* room, GameData* gameData, Resources* resources)
{
	// run the drops manager three times to simulate
	// the lack of checking for vsync in the original 
	// game, making drops fall more often and faster.
	for (int loop = 0; loop < 3; loop++)
	{
		DropsManager_Update(&gameData->dropData, 
							gameData->framebuffer, 
							gameData->cleanBackground, 
							1 /*gameCompletionCount*/,
							resources->sprites_drops);
	}

	// update the number of players and cursor depending on the direction
	// pressed on the controls.
	if (gameData->joystickState.leftPressed)
	{
		gameData->numPlayers = 1;

	}
	else if (gameData->joystickState.rightPressed)
	{
		gameData->numPlayers = 2;
	}

	// draw the cursor
	u16 drawLocation = gameData->numPlayers == 1 ? 0xf64 : 0xf70;  // hardcoded locations in the frambuffer
	u16 eraseLocation = gameData->numPlayers == 1 ? 0xf70 : 0xf64; // based on the original game.

	gameData->framebuffer[drawLocation] = 0xff;// draw the cursor (a white horizontal line)
	gameData->framebuffer[eraseLocation] = 0;  // erase the cursor

	// press button to start
	if (gameData->joystickState.jumpPressed)
	{
		gameData->currentPlayerData = &gameData->playerData[PLAYER_ONE];

		gameData->otherPlayerData = gameData->numPlayers > 1 ? &gameData->playerData[PLAYER_TWO] : NULL;

		for (u8 loop = 0; loop < gameData->numPlayers; loop++)
		{
			Player_GameInit(&gameData->playerData[loop], resources);
		}


		memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);		
		Game_WipeTransitionToRoom(gameData, 0, resources);
	}
}

Room titleScreenRoom =
{
	TITLESCREEN_ROOM_INDEX,
	(InitRoomFunctionType)titleScreen_init,
	(DrawRoomFunctionType)titleScreen_draw,
	(UpdateRoomFunctionType)titleScreen_update
};
