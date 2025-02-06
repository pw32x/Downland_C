#ifndef RESOURCES_INCLUDE_H
#define RESOURCES_INCLUDE_H

#include "../game/base_types.h"

typedef struct
{
    // font
    byte* characterFont;            // 0xd908

    // strings
    byte* text_downland;            // 0xda19
    byte* text_writtenBy;           // 0xda27
    byte* text_michaelAichlmayer;   // 0xda33
    byte* text_copyright1983;       // 0xda45
    byte* text_spectralAssociates;  // 0xda54
    byte* text_licensedTo;          // 0xda68
    byte* text_tandyCorporation;    // 0xda75
    byte* text_allRightsReserved;   // 0xda87
    byte* text_onePlayer;           // 0xda9b
    byte* text_twoPlayer;           // 0xdaa6
    byte* text_highScore;           // 0xdab1
    byte* text_playerOne;           // 0xdabc
    byte* text_playerTwo;           // 0xdac7
    byte* text_pL1;                 // 0xdad2
    byte* text_pL2;                 // 0xdad6
    byte* text_getReadyPlayerOne;   // 0xdada
    byte* text_getReadyPlayerTwo;   // 0xdaef
    byte* text_chamber;             // 0xdb04

    // sprites
    byte* sprites_player;           // 0xdcd6
    byte* collisionmask_player;     // 0xde17
    byte* sprites_bouncyBall;       // 0xde7b
    byte* sprites_bird;             // 0xde9b
    byte* sprite_moneyBag;          // 0xdeb3
    byte* sprite_diamond;           // 0xdec7
    byte* sprite_key;               // 0xd3db
    byte* sprite_playerSplat;       // 0xdeef
    byte* sprite_door;              // 0xdf0a
    byte* sprites_drops;            // 0xdf2a
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



DrawData    
    DrawData_00_Stalactite	0xd5f7
    DrawData_01_WallGoingDown	0xd60c
    DrawData_07_WallPieceGoingUp	0xd616
    DrawData_02_LeftHandCornerPiece	0xd625
    DrawData_08_CornerPieceGoingDownLeft	0xd635
    DrawData_03_TopRightHandCornerPiece	0xd644
    DrawData_04_TopRightHandCornerPiece2	0xd654
    DrawData_05_BottomRightSideOfFloatingPlatforms	0xd663
    DrawData_14_HorizontalRopeStartGoingRight	0xd67b
    DrawData_15_HorizontalRopeEndGoingRight	0xd68d
    DrawData_17_BlankAreaGoingRight	0xd697
    DrawData_18_BlankAreaGoingLeft	0xd6a0
    DrawData_19_BlankAreaGoingDownRight	0xd6a9
    DrawData_0b_ShortLineGoingUp	0xd6b2
    DrawData_0c_VeryShortRope	0xd6d9
    DrawData_0d_ShortRope	0xd6e5
    DrawData_0e_MidLengthRope	0xd6f1
    DrawData_0f_LongRope	0xd6fd
    DrawData_10_VeryLongRope	0xd709
    DrawData_11_SuperLongRope	0xd715
    DrawData_12_ExcessivelyLongRope	0xd721
    DrawData_13_RediculouslyLongRope	0xd72d
    DrawData_PreRope_Maybe	0xd74c
    DrawData_PostRope_Maybe	0xd750  
*/
} Resources;

BOOL Resources_Init(const char* romPath, Resources* resources);
void Resources_Shutdown(Resources* resources);

#endif