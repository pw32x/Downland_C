#include "titlescreen.h"
#include "room_types.h"

#include "../game.h"
#include "../game_data.h"
#include "../drops_manager.h"
#include "../dl_sound.h"
#include "../dl_platform.h"
#include "../joystick_data.h"
#include "resources.h"

void titleScreen_draw(dl_u8 roomNumber);

void titleScreen_init(Room* room)
{
	dl_u8 roomNumber = room->roomNumber;

	// init drops
	DropsManager_Init(&res_roomResources[roomNumber].dropSpawnPositions,
					  roomNumber, 
					  1 /*gameCompletionCount*/);
}

void titleScreen_update(Room* room)
{
	UNUSED(room);

	int loop;

	// run the drops manager three times to simulate
	// the lack of checking for vsync in the original 
	// game, making drops fall more often and faster.
	for (loop = 0; loop < 3; loop++)
	{
		DropsManager_Update(1 /*gameCompletionCount*/);
	}

	// update the number of players and cursor depending on the direction
	// pressed on the controls.
	if (joystickState_leftPressed)
	{
		gameData_numPlayers = 1;

	}
	else if (joystickState_rightPressed)
	{
		gameData_numPlayers = 2;
	}

	// press button to start
	if (joystickState_jumpPressed)
	{
		Game_InitPlayers();
		Game_WipeTransitionToRoom(0);
	}
}

Room titleScreenRoom =
{
	TITLESCREEN_ROOM_INDEX,
	(InitRoomFunctionType)titleScreen_init,
	(DrawRoomFunctionType)titleScreen_draw,
	(UpdateRoomFunctionType)titleScreen_update
};
