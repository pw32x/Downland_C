#ifndef DROPS_MANAGER_INCLUDE_H
#define DROPS_MANAGER_INCLUDE_H

#include "base_types.h"
#include "drops_types.h"

#define DROP_SPRITE_WIDTH 16
#define DROP_SPRITE_ROWS 6
#define DROP_SPRITE_COUNT 1

void DropsManager_Init(DropData* dropData, u8 roomNumber, u8 gameCompletionCount);
void DropsManager_Update(DropData* dropData, u8* framebuffer, u8* cleanBackground, u8 gameCompletionCount, u8* dropSprites);

#endif