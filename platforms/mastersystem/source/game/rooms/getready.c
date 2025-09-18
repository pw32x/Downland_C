#include "room_types.h"

#include "../game.h"
#include "../game_data.h"
#include "../drops_manager.h"
#include "../dl_sound.h"
#include "../dl_platform.h"
#include "../joystick_data.h"
#include "resources.h"

void get_ready_room_draw(dl_u8 roomNumber);

void get_ready_room_init(const Room* room)
{
	UNUSED(room);

	// init drops
	DropsManager_Init(&res_roomResources[TITLESCREEN_ROOM_INDEX].dropSpawnPositions,
					  TITLESCREEN_ROOM_INDEX, 
					  1 /*gameCompletionCount*/);
}

void get_ready_room_update(Room* room)
{
	UNUSED(room);

	int loop;
	dl_u8 roomNumber;

	// run the drops manager three times to simulate
	// the lack of checking for vsync in the original 
	// game, making drops fall more often and faster.
	for (loop = 0; loop < 3; loop++)
	{
		DropsManager_Update(1 /*gameCompletionCount*/);
	}

	// press button to start
	if (joystickState_jumpPressed)
	{
		// enter the new player's room
		roomNumber = 0;
		if (gameData_currentPlayerData->lastDoor != NULL)
			roomNumber = gameData_currentPlayerData->lastDoor->nextRoomNumber;

		Game_WipeTransitionToRoom(roomNumber);
	}
}

Room getReadyRoom =
{
	GET_READY_ROOM_INDEX,
	(InitRoomFunctionType)get_ready_room_init,
	(DrawRoomFunctionType)get_ready_room_draw,
	(UpdateRoomFunctionType)get_ready_room_update
};
