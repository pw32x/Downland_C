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
    u8 characterFont            [SIZE_FROM_RANGE(0xd908, 0xda19)];
                                    
    // string
    u8 text_downland            [SIZE_FROM_RANGE(0xda19, 0xda27)];
    u8 text_writtenBy           [SIZE_FROM_RANGE(0xda27, 0xda33)];
    u8 text_michaelAichlmayer   [SIZE_FROM_RANGE(0xda33, 0xda45)];
    u8 text_copyright1983       [SIZE_FROM_RANGE(0xda45, 0xda54)];
    u8 text_spectralAssociates  [SIZE_FROM_RANGE(0xda54, 0xda68)];
    u8 text_licensedTo          [SIZE_FROM_RANGE(0xda68, 0xda75)];
    u8 text_tandyCorporation    [SIZE_FROM_RANGE(0xda75, 0xda87)];
    u8 text_allRightsReserved   [SIZE_FROM_RANGE(0xda87, 0xda9b)];
    u8 text_onePlayer           [SIZE_FROM_RANGE(0xda9b, 0xdaa6)];
    u8 text_twoPlayer           [SIZE_FROM_RANGE(0xdaa6, 0xdab1)];
    u8 text_highScore           [SIZE_FROM_RANGE(0xdab1, 0xdabc)];
    u8 text_playerOne           [SIZE_FROM_RANGE(0xdabc, 0xdac7)];
    u8 text_playerTwo           [SIZE_FROM_RANGE(0xdac7, 0xdad2)];
    u8 text_pl1                 [SIZE_FROM_RANGE(0xdad2, 0xdad6)];
    u8 text_pl2                 [SIZE_FROM_RANGE(0xdad6, 0xdada)];
    u8 text_getReadyPlayerOne   [SIZE_FROM_RANGE(0xdada, 0xdaef)];
    u8 text_getReadyPlayerTwo   [SIZE_FROM_RANGE(0xdaef, 0xdb04)];
    u8 text_chamber             [SIZE_FROM_RANGE(0xdb04, 0xdb0c)];

    // sprites
    u8 sprites_player          [SIZE_FROM_RANGE(0xdcd7, 0xde17)];
    u8 collisionmasks_player   [SIZE_FROM_RANGE(0xde17, 0xde7b)];
    u8 sprites_bouncyBall      [SIZE_FROM_RANGE(0xde7b, 0xde9b)];
    u8 sprites_bird            [SIZE_FROM_RANGE(0xde9b, 0xdeb3)];
    u8 sprite_moneyBag         [SIZE_FROM_RANGE(0xdeb3, 0xdec7)];
    u8 sprite_diamond          [SIZE_FROM_RANGE(0xdec7, 0xdedb)];
    u8 sprite_key              [SIZE_FROM_RANGE(0xdedb, 0xdeef)];
    u8 sprite_playerSplat      [SIZE_FROM_RANGE(0xdeef, 0xdf0a)];
    u8 sprite_door             [SIZE_FROM_RANGE(0xdf0a, 0xdf2a)];
    u8 sprites_drops           [SIZE_FROM_RANGE(0xdf2a, 0xdf5a)];

    // bit shifted sprites
    u8 bitShiftedSprites_player           [BITSHIFTED_SIZE(PLAYER_SPRITE_COUNT, PLAYER_SPRITE_ROWS)];
    u8 bitShiftedCollisionmasks_player    [BITSHIFTED_SIZE(PLAYER_SPRITE_COUNT, PLAYER_COLLISION_MASK_ROWS)];
    u8 bitShiftedSprites_bouncyBall       [BITSHIFTED_SIZE(BALL_SPRITE_COUNT, BALL_SPRITE_ROWS)];
    u8 bitShiftedSprites_bird             [BITSHIFTED_SIZE(BIRD_SPRITE_COUNT, BIRD_SPRITE_ROWS)];
    u8 bitShiftedSprites_playerSplat      [BITSHIFTED_SIZE(PLAYER_SPLAT_SPRITE_COUNT, PLAYER_SPLAT_SPRITE_ROWS)];
    u8 bitShiftedSprites_door             [BITSHIFTED_SIZE(DOOR_SPRITE_COUNT, DOOR_SPRITE_ROWS)];

    u8* pickupSprites[3];

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

    PickupPosition roomPickupPositions  [SIZE_FROM_RANGE(0xd1ea, 0xd24e) / sizeof(PickupPosition)];
    u8 keyPickUpDoorIndexes             [SIZE_FROM_RANGE(0xd1c2, 0xd1d6)]; // 20 items
    u8 keyPickUpDoorIndexesHardMode     [SIZE_FROM_RANGE(0xd1d6, 0xd1ea)]; // 20 items
    u8 offsetsToDoorsAlreadyActivated   [SIZE_FROM_RANGE(0xceea, 0xcefa)]; // 16 items

    u8* roomsWithBouncingBall;
} Resources;

#endif