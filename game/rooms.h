#ifndef ROOMS_INCLUDE_H
#define ROOMS_INCLUDE_H

#include "base_defines.h"
#include "background_types.h"
#include "resource_types.h"


struct GameData;
struct Room;

typedef struct
{
	u8 roomNumber;
	void (*init)(struct Room* room, struct GameData* gameData, Resources* resources);
	void (*update)(struct GameData* gameData);
} Room;

extern Room* g_rooms[NUM_ROOMS];

typedef void (*InitFunctionType)(struct Room* room, struct GameData* gameObject, Resources* resources);
typedef void (*UpdateFunctionType)(struct GameData* gameObject);

#endif