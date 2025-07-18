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
    BackgroundDrawData backgroundDrawData;
    DropSpawnPositions dropSpawnPositions;
    DoorInfoData doorInfoData;
} RoomResources;

// computes the size from the location of the memory
// addresses in the game rom.
#define SIZE_FROM_RANGE(start, end) (end - start)

#define DESTINATION_BYTES_PER_ROW	3
#define NUM_BIT_SHIFTS 4
#define BITSHIFTED_SIZE(spriteCount, rowCount) (spriteCount * rowCount * DESTINATION_BYTES_PER_ROW * NUM_BIT_SHIFTS)


typedef struct
{
    // font
    const dl_u8* characterFont;

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
    
    //sprites
    const dl_u8* sprites_player;
    const dl_u8* collisionmasks_player;
    const dl_u8* sprites_bouncyBall;
    const dl_u8* sprites_bird;
    const dl_u8* sprite_moneyBag;
    const dl_u8* sprite_diamond;
    const dl_u8* sprite_key;
    const dl_u8* sprite_playerSplat;
    const dl_u8* sprite_door;
    const dl_u8* sprites_drops;

    // bit shifted sprites
    const dl_u8* bitShiftedSprites_player;
    const dl_u8* bitShiftedCollisionmasks_player;
    const dl_u8* bitShiftedSprites_bouncyBall;
    const dl_u8* bitShiftedSprites_bird;
    const dl_u8* bitShiftedSprites_playerSplat;
    const dl_u8* bitShiftedSprites_door;

    const dl_u8* pickupSprites[3];

    // background piece shapes
    ShapeDrawData shapeDrawData_00_Stalactite;	                        // 0xd5f7
    ShapeDrawData shapeDrawData_01_WallGoingDown;	                    // 0xd60c
    ShapeDrawData shapeDrawData_07_WallPieceGoingUp;	                // 0xd616
    ShapeDrawData shapeDrawData_02_LeftHandCornerPiece;	                // 0xd625
    ShapeDrawData shapeDrawData_08_CornerPieceGoingDownLeft;	        // 0xd635
    ShapeDrawData shapeDrawData_03_TopRightHandCornerPiece;	            // 0xd644
    ShapeDrawData shapeDrawData_04_TopRightHandCornerPiece2;	        // 0xd654
    ShapeDrawData shapeDrawData_05_BottomRightSideOfFloatingPlatforms;	// 0xd663
    ShapeDrawData shapeDrawData_14_HorizontalRopeStartGoingRight;	    // 0xd67b
    ShapeDrawData shapeDrawData_15_HorizontalRopeEndGoingRight;	        // 0xd68d
    ShapeDrawData shapeDrawData_17_BlankAreaGoingRight;	                // 0xd697
    ShapeDrawData shapeDrawData_18_BlankAreaGoingLeft;	                // 0xd6a0
    ShapeDrawData shapeDrawData_19_BlankAreaGoingDownRight;	            // 0xd6a9
    ShapeDrawData shapeDrawData_0b_ShortLineGoingUp;	                // 0xd6b2
    ShapeDrawData shapeDrawData_0c_VeryShortRope;	                    // 0xd6d9
    ShapeDrawData shapeDrawData_0d_ShortRope;	                        // 0xd6e5
    ShapeDrawData shapeDrawData_0e_MidLengthRope;	                    // 0xd6f1
    ShapeDrawData shapeDrawData_0f_LongRope;	                        // 0xd6fd
    ShapeDrawData shapeDrawData_10_VeryLongRope;	                    // 0xd709
    ShapeDrawData shapeDrawData_11_SuperLongRope;	                    // 0xd715
    ShapeDrawData shapeDrawData_12_ExcessivelyLongRope;	                // 0xd721
    ShapeDrawData shapeDrawData_13_RediculouslyLongRope;	            // 0xd72d
    ShapeDrawData shapeDrawData_PreRope_Maybe;	                        // 0xd74c
    ShapeDrawData shapeDrawData_PostRope_Maybe;	                        // 0xd750  

    RoomResources roomResources[NUM_ROOMS_PLUS_TITLESCREN];

    const PickupPosition* roomPickupPositions;
    const dl_u8* keyPickUpDoorIndexes;               // 20 items
    const dl_u8* keyPickUpDoorIndexesHardMode;       // 20 items
    const dl_u8* offsetsToDoorsAlreadyActivated;     // 16 items

    const dl_u8* roomsWithBouncingBall; // 10 items
} Resources;

#endif