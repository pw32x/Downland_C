#ifndef ROOMS_INCLUDE_H
#define ROOMS_INCLUDE_H

#include "base_defines.h"
#include "background_types.h"
#include "resource_types.h"

struct GameData;

typedef struct Room
{
	u8 roomNumber;
	void (*init)(struct Room* room, struct GameData* gameData, Resources* resources);
	void (*draw)(u8 roomNumber, u8* framebuffer, Resources* resources);
	void (*update)(struct Room* room, struct GameData* gameData, Resources* resources);
} Room;

extern Room* g_rooms[NUM_ROOMS_AND_ALL];

typedef void (*InitRoomFunctionType)(Room* room, struct GameData* gameObject, Resources* resources);
typedef void (*DrawRoomFunctionType)(u8 roomNumber, u8* framebuffer, Resources* resources);
typedef void (*UpdateRoomFunctionType)(Room* room, struct GameData* gameObject, Resources* resources);


typedef struct
{
	u16 spawnPosition;
	u8 roomNumber;
	u8 globalDoorIndex;
} Door;

typedef struct
{
	u8 doorCount;
	Door* doors;
} Doors;

#endif