#include "base_types.h"
#include "resource_types.h"
#include "custom_background_types.h"

const dl_u8 res_string_downland[14] = { 0x0D, 0x18, 0x20, 0x17, 0x15, 0x0A, 0x17, 0x0D, 0x24, 0x1F, 0x01, 0x26, 0x01, 0xFF };
const dl_u8 res_string_writtenBy[12] = { 0x20, 0x1B, 0x12, 0x1D, 0x1D, 0x0E, 0x17, 0x24, 0x0B, 0x22, 0x25, 0xFF };
const dl_u8 res_string_michaelAichlmayer[18] = { 0x16, 0x12, 0x0C, 0x11, 0x0A, 0x0E, 0x15, 0x24, 0x0A, 0x12, 0x0C, 0x11, 0x15, 0x16, 0x0A, 0x22, 0x1B, 0xFF };
const dl_u8 res_string_copyright1983[15] = { 0x0C, 0x18, 0x19, 0x22, 0x1B, 0x12, 0x10, 0x11, 0x1D, 0x24, 0x01, 0x09, 0x08, 0x03, 0xFF };
const dl_u8 res_string_spectralAssociates[20] = { 0x1C, 0x19, 0x0E, 0x0C, 0x1D, 0x1B, 0x0A, 0x15, 0x24, 0x0A, 0x1C, 0x1C, 0x18, 0x0C, 0x12, 0x0A, 0x1D, 0x0E, 0x1C, 0xFF };
const dl_u8 res_string_licensedTo[13] = { 0x15, 0x12, 0x0C, 0x0E, 0x17, 0x1C, 0x0E, 0x0D, 0x24, 0x1D, 0x18, 0x24, 0xFF };
const dl_u8 res_string_tandyCorporation[18] = { 0x1D, 0x0A, 0x17, 0x0D, 0x22, 0x24, 0x0C, 0x18, 0x1B, 0x19, 0x18, 0x1B, 0x0A, 0x1D, 0x12, 0x18, 0x17, 0xFF };
const dl_u8 res_string_allRightsReserved[20] = { 0x0A, 0x15, 0x15, 0x24, 0x1B, 0x12, 0x10, 0x11, 0x1D, 0x1C, 0x24, 0x1B, 0x0E, 0x1C, 0x0E, 0x1B, 0x1F, 0x0E, 0x0D, 0xFF };
const dl_u8 res_string_onePlayer[11] = { 0x18, 0x17, 0x0E, 0x24, 0x19, 0x15, 0x0A, 0x22, 0x0E, 0x1B, 0xFF };
const dl_u8 res_string_twoPlayer[11] = { 0x1D, 0x20, 0x18, 0x24, 0x19, 0x15, 0x0A, 0x22, 0x0E, 0x1B, 0xFF };
const dl_u8 res_string_highScore[11] = { 0x11, 0x12, 0x10, 0x11, 0x24, 0x1C, 0x0C, 0x18, 0x1B, 0x0E, 0xFF };
const dl_u8 res_string_playerOne[11] = { 0x19, 0x15, 0x0A, 0x22, 0x0E, 0x1B, 0x24, 0x18, 0x17, 0x0E, 0xFF };
const dl_u8 res_string_playerTwo[11] = { 0x19, 0x15, 0x0A, 0x22, 0x0E, 0x1B, 0x24, 0x1D, 0x20, 0x18, 0xFF };
const dl_u8 res_string_pl1[4] = { 0x19, 0x15, 0x01, 0xFF };
const dl_u8 res_string_pl2[4] = { 0x19, 0x15, 0x02, 0xFF };
const dl_u8 res_string_getReadyPlayerOne[21] = { 0x10, 0x0E, 0x1D, 0x24, 0x1B, 0x0E, 0x0A, 0x0D, 0x22, 0x24, 0x19, 0x15, 0x0A, 0x22, 0x0E, 0x1B, 0x24, 0x18, 0x17, 0x0E, 0xFF };
const dl_u8 res_string_getReadyPlayerTwo[21] = { 0x10, 0x0E, 0x1D, 0x24, 0x1B, 0x0E, 0x0A, 0x0D, 0x22, 0x24, 0x19, 0x15, 0x0A, 0x22, 0x0E, 0x1B, 0x24, 0x1D, 0x20, 0x18, 0xFF };
const dl_u8 res_string_chamber[8] = { 0x0C, 0x11, 0x0A, 0x16, 0x0B, 0x0E, 0x1B, 0xFF };


// pick up positions (x: 0 - 127, y: 0 - 191)
const PickupPosition res_roomPickupPositions[50] = 
{
    { 58, 28 },
    { 105, 92 },
    { 101, 32 },
    { 145, 48 },
    { 145, 80 },
    { 35, 52 },
    { 35, 72 },
    { 56, 100 },
    { 151, 60 },
    { 151, 92 },
    { 58, 96 },
    { 68, 28 },
    { 77, 52 },
    { 105, 100 },
    { 120, 64 },
    { 65, 112 },
    { 75, 36 },
    { 108, 80 },
    { 108, 112 },
    { 157, 12 },
    { 63, 12 },
    { 108, 12 },
    { 23, 96 },
    { 150, 60 },
    { 150, 100 },
    { 20, 108 },
    { 48, 4 },
    { 75, 28 },
    { 93, 4 },
    { 155, 80 },
    { 21, 24 },
    { 106, 108 },
    { 43, 64 },
    { 95, 24 },
    { 147, 52 },
    { 48, 12 },
    { 155, 68 },
    { 44, 60 },
    { 61, 108 },
    { 155, 44 },
    { 33, 56 },
    { 52, 32 },
    { 64, 96 },
    { 93, 28 },
    { 124, 96 },
    { 19, 48 },
    { 61, 56 },
    { 68, 24 },
    { 150, 56 },
    { 150, 96 },
};

// pick up door indexes
const dl_u8 res_keyPickUpDoorIndexes[20] = { 1, 3, 30, 28, 19, 6, 11, 10, 21, 26, 22, 23, 24, 25, 27, 33, 29, 255, 31, 32, };

// pick up door indexes (hard mode)
const dl_u8 res_keyPickUpDoorIndexesHardMode[20] = { 1, 3, 30, 28, 6, 19, 10, 11, 21, 26, 23, 22, 25, 24, 33, 27, 29, 255, 32, 31, };

// offests to doors alread activated
const dl_u8 res_offsetsToDoorsAlreadyActivated[16] = { 0, 2, 4, 5, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 20, 255, };

// rooms with the bouncing ball
const dl_u8 res_roomsWithBouncingBall[10] = { 0, 2, 5, 6, 10, 11, 12, 13, 14, 0xff };

extern const dl_u8 chamber0_cleanBackground[6144];
extern const dl_u16 chamber0_tileMap[32 * 24];
const SMSBackgroundData chamber0_customBackgroundData = { chamber0_cleanBackground, chamber0_tileMap};

extern const dl_u8 chamber1_cleanBackground[6144];
extern const dl_u16 chamber1_tileMap[32 * 24];
const SMSBackgroundData chamber1_customBackgroundData = { chamber1_cleanBackground, chamber1_tileMap};

extern const dl_u8 chamber2_cleanBackground[6144];
extern const dl_u16 chamber2_tileMap[32 * 24];
const SMSBackgroundData chamber2_customBackgroundData = { chamber2_cleanBackground, chamber2_tileMap};

extern const dl_u8 chamber3_cleanBackground[6144];
extern const dl_u16 chamber3_tileMap[32 * 24];
const SMSBackgroundData chamber3_customBackgroundData = { chamber3_cleanBackground, chamber3_tileMap};

extern const dl_u8 chamber4_cleanBackground[6144];
extern const dl_u16 chamber4_tileMap[32 * 24];
const SMSBackgroundData chamber4_customBackgroundData = { chamber4_cleanBackground, chamber4_tileMap};

extern const dl_u8 chamber5_cleanBackground[6144];
extern const dl_u16 chamber5_tileMap[32 * 24];
const SMSBackgroundData chamber5_customBackgroundData = { chamber5_cleanBackground, chamber5_tileMap};

extern const dl_u8 chamber6_cleanBackground[6144];
extern const dl_u16 chamber6_tileMap[32 * 24];
const SMSBackgroundData chamber6_customBackgroundData = { chamber6_cleanBackground, chamber6_tileMap};

extern const dl_u8 chamber7_cleanBackground[6144];
extern const dl_u16 chamber7_tileMap[32 * 24];
const SMSBackgroundData chamber7_customBackgroundData = { chamber7_cleanBackground, chamber7_tileMap};

extern const dl_u8 chamber8_cleanBackground[6144];
extern const dl_u16 chamber8_tileMap[32 * 24];
const SMSBackgroundData chamber8_customBackgroundData = { chamber8_cleanBackground, chamber8_tileMap};

extern const dl_u8 chamber9_cleanBackground[6144];
extern const dl_u16 chamber9_tileMap[32 * 24];
const SMSBackgroundData chamber9_customBackgroundData = { chamber9_cleanBackground, chamber9_tileMap};

extern const dl_u8 titleScreen_cleanBackground[6144];
extern const dl_u16 titleScreen_tileMap[32 * 24];
const SMSBackgroundData titleScreen_customBackgroundData = { titleScreen_cleanBackground, titleScreen_tileMap};


extern const dl_u8 getReadyScreen_cleanBackground[6144];

const DropSpawnArea chamber0_dropSpawnPositions_array[7] = 
{
    { 12, 17, 15 }, 
    { 9, 62, 39 }, 
    { 1, 107, 15 }, 
    { 5, 107, 47 }, 
    { 1, 152, 31 }, 
    { 1, 152, 63 }, 
    { 1, 152, 95 }, 
};

const DropSpawnArea chamber1_dropSpawnPositions_array[6] = 
{
    { 5, 17, 15 }, 
    { 4, 17, 79 }, 
    { 5, 62, 15 }, 
    { 1, 62, 79 }, 
    { 1, 107, 103 }, 
    { 2, 152, 71 }, 
};

const DropSpawnArea chamber2_dropSpawnPositions_array[5] = 
{
    { 12, 17, 15 }, 
    { 2, 62, 15 }, 
    { 1, 62, 55 }, 
    { 9, 107, 39 }, 
    { 4, 152, 15 }, 
};

const DropSpawnArea chamber3_dropSpawnPositions_array[6] = 
{
    { 12, 17, 15 }, 
    { 3, 62, 15 }, 
    { 6, 62, 63 }, 
    { 3, 107, 15 }, 
    { 0, 152, 15 }, 
    { 1, 152, 47 }, 
};

const DropSpawnArea chamber4_dropSpawnPositions_array[5] = 
{
    { 12, 17, 15 }, 
    { 1, 62, 15 }, 
    { 8, 77, 31 }, 
    { 1, 152, 47 }, 
    { 1, 152, 87 }, 
};

const DropSpawnArea chamber5_dropSpawnPositions_array[5] = 
{
    { 12, 17, 15 }, 
    { 1, 62, 103 }, 
    { 10, 107, 31 }, 
    { 1, 152, 15 }, 
    { 2, 152, 71 }, 
};

const DropSpawnArea chamber6_dropSpawnPositions_array[2] = 
{
    { 12, 17, 15 }, 
    { 9, 92, 39 }, 
};

const DropSpawnArea chamber7_dropSpawnPositions_array[7] = 
{
    { 5, 17, 15 }, 
    { 1, 32, 63 }, 
    { 4, 17, 79 }, 
    { 0, 92, 39 }, 
    { 0, 92, 87 }, 
    { 0, 107, 47 }, 
    { 0, 107, 79 }, 
};

const DropSpawnArea chamber8_dropSpawnPositions_array[5] = 
{
    { 12, 17, 15 }, 
    { 8, 62, 47 }, 
    { 10, 92, 15 }, 
    { 10, 122, 31 }, 
    { 10, 152, 15 }, 
};

const DropSpawnArea chamber9_dropSpawnPositions_array[3] = 
{
    { 12, 17, 15 }, 
    { 3, 62, 15 }, 
    { 5, 92, 55 }, 
};

const DropSpawnArea titleScreen_dropSpawnPositions_array[1] = 
{
    { 12, 17, 14 }, 
};

const DoorInfo doorInfo0_array[2] = 
{ 
    { 255, 255, 165, 112, 0, 0 },
    { 30, 114, 135, 7, 1, 1 },
};

const DoorInfo doorInfo1_array[4] = 
{ 
    { 135, 5, 30, 112, 0, 2 },
    { 75, 114, 165, 7, 2, 3 },
    { 30, 114, 120, 7, 2, 4 },
    { 30, 5, 120, 112, 6, 26 },
};

const DoorInfo doorInfo2_array[6] = 
{ 
    { 165, 5, 75, 112, 1, 5 },
    { 120, 5, 30, 112, 1, 6 },
    { 120, 114, 120, 7, 3, 7 },
    { 75, 114, 75, 7, 3, 19 },
    { 30, 114, 30, 7, 3, 18 },
    { 30, 5, 135, 112, 5, 21 },
};

const DoorInfo doorInfo3_array[7] = 
{ 
    { 120, 5, 120, 112, 2, 8 },
    { 165, 114, 165, 7, 4, 9 },
    { 120, 114, 120, 7, 4, 10 },
    { 75, 5, 75, 112, 2, 20 },
    { 75, 114, 75, 7, 4, 11 },
    { 30, 5, 30, 112, 2, 17 },
    { 30, 114, 30, 7, 4, 16 },
};

const DoorInfo doorInfo4_array[4] = 
{ 
    { 165, 5, 165, 112, 3, 12 },
    { 120, 5, 120, 112, 3, 13 },
    { 75, 5, 75, 112, 3, 14 },
    { 30, 5, 30, 112, 3, 15 },
};

const DoorInfo doorInfo5_array[3] = 
{ 
    { 135, 114, 30, 7, 2, 22 },
    { 165, 5, 60, 112, 6, 23 },
    { 75, 114, 165, 7, 7, 26 },
};

const DoorInfo doorInfo6_array[2] = 
{ 
    { 120, 114, 30, 7, 1, 25 },
    { 60, 114, 165, 7, 5, 24 },
};

const DoorInfo doorInfo7_array[2] = 
{ 
    { 165, 5, 75, 112, 5, 27 },
    { 75, 114, 165, 7, 8, 28 },
};

const DoorInfo doorInfo8_array[2] = 
{ 
    { 165, 5, 75, 112, 7, 29 },
    { 45, 114, 30, 7, 9, 30 },
};

const DoorInfo doorInfo9_array[3] = 
{ 
    { 150, 5, 165, 112, 8, 32 },
    { 30, 5, 45, 112, 8, 31 },
    { 165, 114, 165, 112, 0, 33 },
};

const RoomResources res_roomResources[NUM_ROOMS_PLUS_TITLESCREN] = 
{
    { (const dl_u8*)&chamber0_customBackgroundData, { 7, chamber0_dropSpawnPositions_array }, { 2, doorInfo0_array } },
    { (const dl_u8*)&chamber1_customBackgroundData, { 6, chamber1_dropSpawnPositions_array }, { 4, doorInfo1_array } },
    { (const dl_u8*)&chamber2_customBackgroundData, { 5, chamber2_dropSpawnPositions_array }, { 6, doorInfo2_array } },
    { (const dl_u8*)&chamber3_customBackgroundData, { 6, chamber3_dropSpawnPositions_array }, { 7, doorInfo3_array } },
    { (const dl_u8*)&chamber4_customBackgroundData, { 5, chamber4_dropSpawnPositions_array }, { 4, doorInfo4_array } },
    { (const dl_u8*)&chamber5_customBackgroundData, { 5, chamber5_dropSpawnPositions_array }, { 3, doorInfo5_array } },
    { (const dl_u8*)&chamber6_customBackgroundData, { 2, chamber6_dropSpawnPositions_array }, { 2, doorInfo6_array } },
    { (const dl_u8*)&chamber7_customBackgroundData, { 7, chamber7_dropSpawnPositions_array }, { 2, doorInfo7_array } },
    { (const dl_u8*)&chamber8_customBackgroundData, { 5, chamber8_dropSpawnPositions_array }, { 2, doorInfo8_array } },
    { (const dl_u8*)&chamber9_customBackgroundData, { 3, chamber9_dropSpawnPositions_array }, { 3, doorInfo9_array } },
    { (const dl_u8*)&titleScreen_customBackgroundData, { 1, titleScreen_dropSpawnPositions_array }, { 0, NULL } },
};

