#ifndef DROPS_MANAGER_INCLUDE_H
#define DROPS_MANAGER_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"
#include "drops_types.h"

#define DROP_SPRITE_WIDTH 16
#define DROP_SPRITE_ROWS 6
#define DROP_SPRITE_COUNT 1

void DropsManager_Init(const DropSpawnPositions* dropSpawnPositions,
					   dl_u8 roomNumber, 
					   dl_u8 gameCompletionCount);

void DropsManager_Update(dl_u8 gameCompletionCount);

extern Drop dropData_drops[NUM_DROPS];
extern const DropSpawnPositions* dropData_dropSpawnPositions;
extern dl_u8 dropData_activeDropsCount;

#endif