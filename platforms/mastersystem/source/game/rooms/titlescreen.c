#include "titlescreen.h"
#include "room_types.h"

#include "../game_types.h"
#include "../draw_utils.h"
#include "../drops_manager.h"
#include "../dl_sound.h"
#include "../dl_platform.h"

#ifndef CUSTOM_ROOM_DRAW
void titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	convertScoreToString(gameData->playerData[PLAYER_ONE].score, gameData->playerData[PLAYER_ONE].scoreString);

	convertScoreToString(gameData->playerData[PLAYER_TWO].score, gameData->playerData[PLAYER_TWO].scoreString);

	if (gameData->playerData[PLAYER_ONE].score > gameData->highScore)
		gameData->highScore = gameData->playerData[PLAYER_ONE].score;
	else if (gameData->playerData[PLAYER_TWO].score > gameData->highScore)
		gameData->highScore = gameData->playerData[PLAYER_TWO].score;

	convertScoreToString(gameData->highScore, gameData->string_highScore);

#ifndef DISABLE_FRAMEBUFFER
	dl_u8* cleanBackground = gameData->cleanBackground;


	// init background and text
	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
				   resources,
				   cleanBackground);

	// title screen text
	drawText(resources->text_downland, resources->characterFont, cleanBackground, 0x03c9); // 0x07c9 original coco mem location
	drawText(resources->text_writtenBy, resources->characterFont, cleanBackground, 0x050a); // 0x090A original coco mem location
	drawText(resources->text_michaelAichlmayer, resources->characterFont, cleanBackground, 0x647); // 0x0A47 original coco mem location
	drawText(resources->text_copyright1983, resources->characterFont, cleanBackground, 0x789); // 0x0B89 original coco mem location
	drawText(resources->text_spectralAssociates, resources->characterFont, cleanBackground, 0x8c6); // 0x0CC6 original coco mem location
	drawText(resources->text_licensedTo, resources->characterFont, cleanBackground, 0xa0a); // 0x0E0A original coco mem location
	drawText(resources->text_tandyCorporation, resources->characterFont, cleanBackground, 0xb47); // 0x0F47 original coco mem location
	drawText(resources->text_allRightsReserved, resources->characterFont, cleanBackground, 0xc86); // 0x1086 original coco mem location
	drawText(resources->text_onePlayer, resources->characterFont, cleanBackground, 0xf05); // 0x1305 original coco mem location
	drawText(resources->text_twoPlayer, resources->characterFont, cleanBackground, 0xf11); // 0x1311 original coco mem location
	drawText(resources->text_highScore, resources->characterFont, cleanBackground, 0x118b); // 0x158B original coco mem location
	drawText(resources->text_playerOne, resources->characterFont, cleanBackground, 0x1406); // 0x1806 original coco mem location
	drawText(resources->text_playerTwo, resources->characterFont, cleanBackground, 0x1546); // 0x1946 original coco mem location



	drawText(gameData->playerData[PLAYER_ONE].scoreString, 
			 resources->characterFont, 
			 cleanBackground, 
			 TITLESCREEN_PLAYERONE_SCORE_LOCATION);



	drawText(gameData->playerData[PLAYER_TWO].scoreString, 
			 resources->characterFont, 
			 cleanBackground, 
			 TITLESCREEN_PLAYERTWO_SCORE_LOCATION);



	drawText(gameData->string_highScore, 
			 resources->characterFont, 
			 cleanBackground, 
			 TITLESCREEN_HIGHSCORE_LOCATION);
#endif
}
#else
void titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources);
#endif

void titleScreen_init(Room* room, GameData* gameData, const Resources* resources)
{
	dl_u8 roomNumber = room->roomNumber;

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[roomNumber].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, roomNumber, 1 /*gameCompletionCount*/);
}

void titleScreen_update(Room* room, GameData* gameData, const Resources* resources)
{
	UNUSED(room);

	int loop;
#ifndef DISABLE_FRAMEBUFFER
	dl_u16 drawLocation;
	dl_u16 eraseLocation;
#endif

	// run the drops manager three times to simulate
	// the lack of checking for vsync in the original 
	// game, making drops fall more often and faster.
	for (loop = 0; loop < 3; loop++)
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

#ifndef DISABLE_FRAMEBUFFER
	// draw the cursor
	drawLocation = gameData->numPlayers == 1 ? 0xf64 : 0xf70;  // hardcoded locations in the frambuffer
	eraseLocation = gameData->numPlayers == 1 ? 0xf70 : 0xf64; // based on the original game.

	gameData->framebuffer[drawLocation] = 0xff;// draw the cursor (a white horizontal line)
	gameData->framebuffer[eraseLocation] = 0;  // erase the cursor
#endif

	// press button to start
	if (gameData->joystickState.jumpPressed)
	{
		Game_InitPlayers(gameData, resources);

#ifndef DISABLE_FRAMEBUFFER
		dl_memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);		
#endif
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
