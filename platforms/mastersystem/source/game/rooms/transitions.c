#include "room_types.h"

#include "../drops_manager.h"
#include "../dl_sound.h"
#include "../dl_platform.h"

void transition_init(Room* targetRoom);
void transition_update(Room* room);

const Room transitionRoom =
{
	TRANSITION_ROOM_INDEX,
	(InitRoomFunctionType)transition_init,
	NULL, // don't draw anything. 
	(UpdateRoomFunctionType)transition_update
};

void wipe_transition_init(Room* targetRoom);
void wipe_transition_update(Room* room);

const Room wipeTransitionRoom =
{
	WIPE_TRANSITION_ROOM_INDEX,
	(InitRoomFunctionType)wipe_transition_init,
	NULL,
	(UpdateRoomFunctionType)wipe_transition_update
};
