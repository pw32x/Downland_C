#ifndef RESOURCE_TYPES_INCLUDE_H
#define RESOURCE_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"
#include "background_types.h"
#include "pickup_types.h"
#include "door_types.h"
#include "drops_types.h"

typedef struct
{
    const dl_u8* backgroundDrawData;
    DropSpawnPositions dropSpawnPositions;
    DoorInfoData doorInfoData;
} RoomResources;

#endif