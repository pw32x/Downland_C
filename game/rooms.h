#ifndef ROOMS_INCLUDE_H
#define ROOMS_INCLUDE_H

#include "background_types.h"

struct GameData;

typedef struct
{
	const BackgroundDrawData* backgroundDrawData;
	void (*init)(struct GameData* gameData);
	void (*update)(struct GameData* gameData);
} Room;

#define NUM_ROOMS 10

extern Room g_titleScreenRoom;
extern Room* g_rooms[NUM_ROOMS];

typedef void (*InitFunctionType)(struct GameData* gameObject);
typedef void (*UpdateFunctionType)(struct GameData* gameObject);

#endif