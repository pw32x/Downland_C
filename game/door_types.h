#ifndef DOOR_TYPES_INCLUDE_H
#define DOOR_TYPES_INCLUDE_H

#include "base_types.h"

#define DOOR_SPRITE_COUNT           1
#define DOOR_SPRITE_ROWS            16
#define DOOR_SPRITE_BYTES_PER_ROW   2
#define DOOR_BITSHIFTED_SPRITE_FRAME_SIZE (DOOR_SPRITE_ROWS * 3) // rows * 3 bytes per row

#define DOOR_TOTAL_COUNT 0x22 // 34

// per-room door information stored in rom
typedef struct
{
    u8 y;
    u8 x;
    u8 yLocationInNextRoom;
    u8 xLocationInNextRoom;
    u8 nextRoomNumber;
    u8 globalDoorIndex;
} DoorInfo;

typedef struct
{
    u8 drawInfosCount;
    DoorInfo* doorInfos;
} DoorInfoData;

#endif