#ifndef DROPS_TYPES_INCLUDE_H
#define DROPS_TYPES_INCLUDE_H

#include "base_types.h"

#define NUM_DROPS 10

#define DROP_WIDTH  4
#define DROP_HEIGHT	6

typedef struct
{
	u8 wiggleTimer;
	u16 speedY;
	u16 y; // u16 because 256 pixels with 256 subpixels resolution
	u8 x; // position in half resolution (0 - 127)
	u8* spriteData;
	u8 collisionMask;
} Drop;

typedef struct
{
	u8 dropSpawnPointsCount;
	u8 y;
	u8 x;
} DropSpawnArea;

typedef struct
{
	u8 spawnAreasCount;
	DropSpawnArea* dropSpawnAreas;
} DropSpawnPositions;

typedef struct
{
	Drop drops[NUM_DROPS];
	DropSpawnPositions* dropSpawnPositions;
	u8 activeDropsCount;
} DropData;

#endif