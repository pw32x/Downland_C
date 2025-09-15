#include "room_types.h"

#include "../game_types.h"
#include "../draw_utils.h"
#include "../drops_manager.h"
#include "../dl_sound.h"
#include "../dl_platform.h"

#ifndef CUSTOM_ROOM_DRAW
void get_ready_room_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
#ifndef DISABLE_FRAMEBUFFER
	const dl_u8* getReadyString;

	dl_u8* cleanBackground = gameData->cleanBackground;


	// init background and text
	drawBackground(&resources->roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData, 
				   resources,
				   cleanBackground);

	// get ready text
	getReadyString = gameData->currentPlayerData->playerNumber == PLAYER_ONE ? resources->text_getReadyPlayerOne : resources->text_getReadyPlayerTwo;
	drawText(getReadyString, resources->characterFont, cleanBackground, 0x0b66);
#endif
}
#else
void get_ready_room_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources);
#endif

void get_ready_room_init(Room* room, GameData* gameData, const Resources* resources)
{
	UNUSED(room);

	// init drops
	gameData->dropData.dropSpawnPositions = &resources->roomResources[TITLESCREEN_ROOM_INDEX].dropSpawnPositions;
	DropsManager_Init(&gameData->dropData, TITLESCREEN_ROOM_INDEX, 1 /*gameCompletionCount*/);
}

void get_ready_room_update(Room* room, GameData* gameData, const Resources* resources)
{
	UNUSED(room);

	int loop;
	dl_u8 roomNumber;

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

	// press button to start
	if (gameData->joystickState.jumpPressed)
	{
		// enter the new player's room
		roomNumber = 0;
		if (gameData->currentPlayerData->lastDoor != NULL)
			roomNumber = gameData->currentPlayerData->lastDoor->nextRoomNumber;

#ifndef DISABLE_FRAMEBUFFER
		dl_memset(gameData->framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);		
#endif
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
