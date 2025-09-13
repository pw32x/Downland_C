#ifndef DROPS_TYPES_INCLUDE_H
#define DROPS_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

#define NUM_DROPS 10

#define DROP_WIDTH  2
#define DROP_COLLISION_WIDTH  2
#define DROP_HEIGHT	6

typedef DL_PACKED struct
{
	dl_u8 wiggleTimer;
	dl_u16 speedY;
	dl_u16 y; // dl_u16 because 256 pixels with 256 subpixels resolution
	dl_u8 x; // position in half resolution (0 - 127)
	const dl_u8* spriteData;
	dl_u8 collisionMask;
} Drop;

typedef DL_PACKED struct
{
	dl_u8 dropSpawnPointsCount;
	dl_u8 y;
	dl_u8 x;
} DropSpawnArea;

typedef DL_PACKED struct
{
	dl_u8 spawnAreasCount;
	const DropSpawnArea* dropSpawnAreas;
} DropSpawnPositions;

typedef DL_PACKED struct
{
	Drop drops[NUM_DROPS];
	const DropSpawnPositions* dropSpawnPositions;
	dl_u8 activeDropsCount;
} DropData;

#endif