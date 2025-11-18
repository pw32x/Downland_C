#ifndef DOOR_TYPES_INCLUDE_H
#define DOOR_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

#define DOOR_SPRITE_COUNT           1
#define DOOR_SPRITE_WIDTH           16
#define DOOR_SPRITE_ROWS            16
#define DOOR_SPRITE_BYTES_PER_ROW   2
#define DOOR_BITSHIFTED_SPRITE_FRAME_SIZE (DOOR_SPRITE_ROWS * 3) // rows * 3 bytes per row

#define LAST_DOOR_INDEX 0x21 // 33

#define DOOR_TOTAL_COUNT 0x22 // 34

// per-room door information stored in rom
typedef struct
{
    dl_u8 y;
    dl_u8 x;
    dl_u8 yLocationInNextRoom;
    dl_u8 xLocationInNextRoom;
    dl_u8 nextRoomNumber;
    dl_u8 globalDoorIndex;
} DoorInfo;

typedef struct
{
    dl_u8 drawInfosCount;
    const DoorInfo* doorInfos;
} DoorInfoData;

#endif