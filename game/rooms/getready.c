#include "room_types.h"

#include <string.h>

#include "..\game_types.h"
#include "..\draw_utils.h"
#include "..\drops_manager.h"
#include "..\sound.h"

void get_ready_room_draw(u8 roomNumber, GameData* gameData, Resources* resources)
{
	u8* framebuffer = gameData->cleanBackground;

	// init background and text
	drawBackground(&resources->roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData, 
				   resources,
				   framebuffer);

	// get ready text
	u8* getReadyString = gameData->currentPlayerData->playerNumber == PLAYER_ONE ? resources->text_getReadyPlayerOne : resources->text_getReadyPlayerTwo;

	drawText(getReadyString, resources->characterFont, framebuffer, 0x0b66);
}

void get_ready_room_init(Room* room, GameData* gameData, Resources* resources)
{
	u8 roomNumber = room->roomNumber;
	gameData->targetFps = NORMAL_FPS * 3; // speed up the game, like the title screen 

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[TITLESCREEN_ROOM_INDEX].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, TITLESCREEN_ROOM_INDEX, 1 /*gameCompletionCount*/);
}

void get_ready_room_update(Room* room, GameData* gameData, Resources* resources)
{
	DropsManager_Update(&gameData->dropData, 
						gameData->framebuffer, 
						gameData->cleanBackground, 
						1 /*gameCompletionCount*/,
						resources->sprites_drops);


	// press button to start
	if (gameData->joystickState.jumpPressed)
	{
		// enter the new player's room
		u8 roomNumber = 0;
		if (gameData->currentPlayerData->lastDoor != NULL)
			roomNumber = gameData->currentPlayerData->lastDoor->nextRoomNumber;

		gameData->targetFps = NORMAL_FPS;
		memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);		
		Game_WipeTransitionToRoom(gameData, roomNumber, resources);
	}
}

Room getReadyRoom =
{
	GET_READY_ROOM_INDEX,
	(InitRoomFunctionType)get_ready_room_init,
	(DrawRoomFunctionType)get_ready_room_draw,
	(UpdateRoomFunctionType)get_ready_room_update
};
