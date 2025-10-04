#ifndef ROOM_TYPES_INCLUDE_H
#define ROOM_TYPES_INCLUDE_H

#include "../base_defines.h"
#include "../resource_types.h"

// A room generally means one of the ten chambers of the game but in
// reality it means the game's current mode. The game can be in a 
// gameplay mode, a titlescreen/getready mode, or a transition mode.
// A mode needs an init, an update, and a draw. 

typedef struct Room
{
	dl_u8 roomNumber;
	void (*init)(const struct Room* room);
	void (*draw)(dl_u8 roomNumber);
	void (*update)(struct Room* room);
} Room;

typedef void (*InitRoomFunctionType)(Room* room);
typedef void (*DrawRoomFunctionType)(dl_u8 roomNumber);
typedef void (*UpdateRoomFunctionType)(Room* room);


#endif