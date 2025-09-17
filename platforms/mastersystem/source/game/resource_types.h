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

// computes the size from the location of the memory
// addresses in the game rom.
#define SIZE_FROM_RANGE(start, end) (end - start)

#define DESTINATION_BYTES_PER_ROW	3
#define NUM_BIT_SHIFTS 4
#define BITSHIFTED_SIZE(spriteCount, rowCount) (spriteCount * rowCount * DESTINATION_BYTES_PER_ROW * NUM_BIT_SHIFTS)

#define CHARACTER_FONT_COUNT    39
#define CHARACTER_FONT_WIDTH    8
#define CHARACTER_FONT_HEIGHT   7

typedef struct
{
    //strings
    const dl_u8* text_downland;
    const dl_u8* text_writtenBy;
    const dl_u8* text_michaelAichlmayer;
    const dl_u8* text_copyright1983;
    const dl_u8* text_spectralAssociates;
    const dl_u8* text_licensedTo;
    const dl_u8* text_tandyCorporation;
    const dl_u8* text_allRightsReserved;
    const dl_u8* text_onePlayer;
    const dl_u8* text_twoPlayer;
    const dl_u8* text_highScore;
    const dl_u8* text_playerOne;
    const dl_u8* text_playerTwo;
    const dl_u8* text_pl1;
    const dl_u8* text_pl2;
    const dl_u8* text_getReadyPlayerOne;
    const dl_u8* text_getReadyPlayerTwo;
    const dl_u8* text_chamber;

    RoomResources roomResources[NUM_ROOMS_PLUS_TITLESCREN];

    const PickupPosition* roomPickupPositions;
    const dl_u8* keyPickUpDoorIndexes;               // 20 items
    const dl_u8* keyPickUpDoorIndexesHardMode;       // 20 items
    const dl_u8* offsetsToDoorsAlreadyActivated;     // 16 items

    const dl_u8* roomsWithBouncingBall; // 10 items
} Resources;

#endif