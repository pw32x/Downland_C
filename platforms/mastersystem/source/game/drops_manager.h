#ifndef DROPS_MANAGER_INCLUDE_H
#define DROPS_MANAGER_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"
#include "drops_types.h"

#define DROP_SPRITE_WIDTH 16
#define DROP_SPRITE_ROWS 6
#define DROP_SPRITE_COUNT 1

void DropsManager_Init(DropData* dropData, 
					   dl_u8 roomNumber, 
					   dl_u8 gameCompletionCount);

void DropsManager_Update(DropData* dropData, 
						 dl_u8* framebuffer, 
						 dl_u8* cleanBackground, 
						 dl_u8 gameCompletionCount, 
						 const dl_u8* dropSprites);

#endif