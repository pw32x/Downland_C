#include <stdio.h>
#include "SMSlib.h"

#include "base_types.h"
#include "game_types.h"

#define VDP_ASSETS_BANK 2
#define CHAMBER0_BANK 3
#define CHAMBER1_BANK 4

void* dl_alloc(dl_u32 size)
{
	return NULL;
}

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{

}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

void Sound_StopAll(void)
{
}


#define TRUE 1
#define FALSE 0

dl_u8 downlandPalette[] = 
{
	0x00, 0x30, 0x0b, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

extern unsigned char const tileSet4bpp[6240];
extern unsigned char const characterFont4bpp[1248];
extern const dl_u16 chamber0_tileMap[];
extern const dl_u16 chamber1_tileMap[];

extern unsigned char const ball4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const bird4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const diamond4bpp[128];  // 4 tiles x 32 bytes
extern unsigned char const door4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const drop4bpp[64]; // 2 tiles x 32 bytes
extern unsigned char const key4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const moneyBag4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const player4bpp[1280]; // 40 tiles x 32 bytes

void Scroll_InitTilemap(void)
{
	SMS_mapROMBank(CHAMBER0_BANK);
	SMS_loadTileMap(0, 0, chamber0_tileMap, 32 * 24 * 2);
}

void chamber_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
}

void get_ready_room_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
}

void titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{

}

GameData gameData;
Resources resources;

void buildResources()
{
    /*
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
    //ShapeDrawData shapeDrawData_00_Stalactite;	                        // 0xd5f7
    //ShapeDrawData shapeDrawData_01_WallGoingDown;	                    // 0xd60c
    //ShapeDrawData shapeDrawData_07_WallPieceGoingUp;	                // 0xd616
    //ShapeDrawData shapeDrawData_02_LeftHandCornerPiece;	                // 0xd625
    //ShapeDrawData shapeDrawData_08_CornerPieceGoingDownLeft;	        // 0xd635
    //ShapeDrawData shapeDrawData_03_TopRightHandCornerPiece;	            // 0xd644
    //ShapeDrawData shapeDrawData_04_TopRightHandCornerPiece2;	        // 0xd654
    //ShapeDrawData shapeDrawData_05_BottomRightSideOfFloatingPlatforms;	// 0xd663
    //ShapeDrawData shapeDrawData_14_HorizontalRopeStartGoingRight;	    // 0xd67b
    //ShapeDrawData shapeDrawData_15_HorizontalRopeEndGoingRight;	        // 0xd68d
    //ShapeDrawData shapeDrawData_17_BlankAreaGoingRight;	                // 0xd697
    //ShapeDrawData shapeDrawData_18_BlankAreaGoingLeft;	                // 0xd6a0
    //ShapeDrawData shapeDrawData_19_BlankAreaGoingDownRight;	            // 0xd6a9
    //ShapeDrawData shapeDrawData_0b_ShortLineGoingUp;	                // 0xd6b2
    //ShapeDrawData shapeDrawData_0c_VeryShortRope;	                    // 0xd6d9
    //ShapeDrawData shapeDrawData_0d_ShortRope;	                        // 0xd6e5
    //ShapeDrawData shapeDrawData_0e_MidLengthRope;	                    // 0xd6f1
    //ShapeDrawData shapeDrawData_0f_LongRope;	                        // 0xd6fd
    //ShapeDrawData shapeDrawData_10_VeryLongRope;	                    // 0xd709
    //ShapeDrawData shapeDrawData_11_SuperLongRope;	                    // 0xd715
    //ShapeDrawData shapeDrawData_12_ExcessivelyLongRope;	                // 0xd721
    //ShapeDrawData shapeDrawData_13_RediculouslyLongRope;	            // 0xd72d
    //ShapeDrawData shapeDrawData_PreRope_Maybe;	                        // 0xd74c
    //ShapeDrawData shapeDrawData_PostRope_Maybe;	                        // 0xd750  

    RoomResources roomResources[NUM_ROOMS_PLUS_TITLESCREN];

    const PickupPosition* roomPickupPositions;
    const dl_u8* keyPickUpDoorIndexes;               // 20 items
    const dl_u8* keyPickUpDoorIndexesHardMode;       // 20 items
    const dl_u8* offsetsToDoorsAlreadyActivated;     // 16 items

    const dl_u8* roomsWithBouncingBall; // 10 items
    */
}

void main(void)
{
	buildResources();

	/* Clear VRAM */
	SMS_VRAMmemsetW(0, 0x0000, 16384);

	SMS_loadBGPalette(downlandPalette);
	SMS_loadSpritePalette(downlandPalette);
  
	// load tiles for background
	SMS_mapROMBank(VDP_ASSETS_BANK);
	SMS_loadTiles(ball4bpp, 256, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(bird4bpp, 256 + 4, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(diamond4bpp, 256 + 8, 128);  // 4 tiles x 32 bytes
	SMS_loadTiles(door4bpp, 256 + 12, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(drop4bpp, 256 + 16, 64); // 2 tiles x 32 bytes
	SMS_loadTiles(key4bpp, 256 + 18, 1280); // 4 tiles x 32 bytes
	SMS_loadTiles(moneyBag4bpp, 256 + 22, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(player4bpp, 256 + 64, 1280); // 40 tiles x 32 bytes

	SMS_loadTiles(tileSet4bpp, 0, 6240);
	SMS_loadTiles(characterFont4bpp, 195, 1248);
  
	Scroll_InitTilemap();

	/* Turn on the display */
	SMS_displayOn();
	SMS_waitForVBlank ();

	for(;;) 
	{ 
		// Game Loop
		SMS_initSprites();

		// VBLANK
		SMS_waitForVBlank ();

		SMS_copySpritestoSAT();
	}
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"pw","basicsmsproject","A basic SMS example project with devkitSMS");
