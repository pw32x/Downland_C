#ifndef RESOURCE_TYPES_INCLUDE_H
#define RESOURCE_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"
#include "background_types.h"
#include "pickup_types.h"
#include "drops_types.h"

typedef struct
{
    BackgroundDrawData backgroundDrawData;
    DropSpawnPositions dropSpawnPositions;
} RoomResources;

typedef struct
{
    // font
    u8* characterFont;                  // 0xd908

    // strings
    u8* text_downland;                  // 0xda19
    u8* text_writtenBy;                 // 0xda27
    u8* text_michaelAichlmayer;         // 0xda33
    u8* text_copyright1983;             // 0xda45
    u8* text_spectralAssociates;        // 0xda54
    u8* text_licensedTo;                // 0xda68
    u8* text_tandyCorporation;          // 0xda75
    u8* text_allRightsReserved;         // 0xda87
    u8* text_onePlayer;                 // 0xda9b
    u8* text_twoPlayer;                 // 0xdaa6
    u8* text_highScore;                 // 0xdab1
    u8* text_playerOne;                 // 0xdabc
    u8* text_playerTwo;                 // 0xdac7
    u8* text_pl1;                       // 0xdad2
    u8* text_pl2;                       // 0xdad6
    u8* text_getReadyPlayerOne;         // 0xdada
    u8* text_getReadyPlayerTwo;         // 0xdaef
    u8* text_chamber;                   // 0xdb04

    // sprites
    u8* sprites_player;                 // 0xdcd6
    u8* collisionmask_player;           // 0xde17
    u8* sprites_bouncyBall;             // 0xde7b
    u8* sprites_bird;                   // 0xde9b
    u8* sprite_moneyBag;                // 0xdeb3
    u8* sprite_diamond;                 // 0xdec7
    u8* sprite_key;                     // 0xd3db
    u8* sprite_playerSplat;             // 0xdeef
    u8* sprite_door;                    // 0xdf0a
    u8* sprites_drops;                  // 0xdf2a

    // bit shifted sprites
    u8* bitShiftedSprites_player;
    u8* bitShiftedSprites_bouncyBall;
    u8* bitShiftedSprites_bird;

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

    PickupPosition* roomPickupPositions;

    u8* keyPickUpDoorIndexes; // 20 items
    u8* keyPickUpDoorIndexesHardMode; // 20 items

    u8* roomsWithBouncingBall;
/*
Sprites 0xdcd7
    PlayerSprite_Right_Stand 0xdcd6
    PlayerSprite_Right_Run0 0xdcf7
    PlayerSprite_Right_Run1 0xdd17
    PlayerSprite_Right_Run2 0xdd37
    PlayerSprite_Right_Climb 0xdd57
    PlayerSprite_Left_Climb 0xdd77
    PlayerSprite_Left_Stand 0xdd97
    PlayerSprite_Left_Run0 0xddb7
    PlayerSprite_Left_Run1 0xddd7
    PlayerSprite_Left_Run2 0xddf7
    Player_CollisionMasks 0xde17
    BouncyBallSprite1 0xde7b
    BouncyBallSprite2 0xde8b
    BirdSprite1 0xde9b
    BirdSprite2 0xdea7
    MoneyBagSprite 0xdeb3
    DiamondSprite 0xdec7
    KeySprite 0xd3db
    PlayerSplatSprite 0xdeef
    DoorSprite 0xdf0a
    DropSprites 0xdf2a

Room Data

    RoomGraphicsAndDoorData 0xd25a
        Room0_GraphcisAndDoorData   0xd25a
        Room1_GraphcisAndDoorData   0xd27d
        Room2_GraphcisAndDoorData   0xd298
        Room3_GraphcisAndDoorData   0xd2bf
        Room4_GraphcisAndDoorData   0xd2ec
        Room5_GraphcisAndDoorData   0xd307
        Room6_GraphcisAndDoorData   0xd31c
        Room7_GraphcisAndDoorData   0xd32b
        Room8_GraphcisAndDoorData   0xd33a
        Room9_GraphcisAndDoorData   0xd349


*/
} Resources;

#endif