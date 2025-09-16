#include "titlescreen.h"
#include "room_types.h"

#include "../game_types.h"
#include "../draw_utils.h"
#include "../drops_manager.h"
#include "../dl_sound.h"
#include "../dl_platform.h"

void titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources);

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

	// run the drops manager three times to simulate
	// the lack of checking for vsync in the original 
	// game, making drops fall more often and faster.
	for (loop = 0; loop < 3; loop++)
	{
		DropsManager_Update(&gameData->dropData, 
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

	// press button to start
	if (gameData->joystickState.jumpPressed)
	{
		Game_InitPlayers(gameData, resources);
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
