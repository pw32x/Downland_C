#include "game_runner.h"

#include <genesis.h>
#include "kdebug.h"

#include "tilemaps.h"
#define TILE_MAP_WIDTH (FRAMEBUFFER_WIDTH / 8)
#define TILE_MAP_HEIGHT (FRAMEBUFFER_HEIGHT / 8)
#define TILE_MAP_SIZE (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)

// generated headers
#include "tileset.h"

/*
#include <gba_sprites.h>
#include <gba_video.h>
#include <gba_systemcalls.h>

#include <string.h>
*/

/*
#include "image_utils.h"
#include "gba_defines.h"
#include "transition_effect.h"
*/

#include "drops_manager.h"
#include "draw_utils.h"
#include "rooms\chambers.h"
#include "rooms\titlescreen.h"
#include "dl_rand.h"

/*
typedef struct
{
	dl_u16 attr0;
	dl_u16 attr1;
	dl_u16 attr2;
} SpriteAttributes;

SpriteAttributes g_8x8SpriteAttributes;
SpriteAttributes g_16x8SpriteAttributes;
SpriteAttributes g_16x16SpriteAttributes;
SpriteAttributes g_hudTextSpriteAttributes;
SpriteAttributes g_textSpriteAttributes;
SpriteAttributes g_playerIconSpriteAttributes;
SpriteAttributes g_32x16SpriteAttributes;

#define SCROLL_MAX_X 16
#define SCROLL_MAX_Y 32

dl_s16 g_scrollX;
dl_s16 g_scrollY;

dl_u16 g_oldPlayerX = 0;
dl_u16 g_oldPlayerY = 0;
*/


typedef struct
{
	//const SpriteAttributes* spriteAttributes;
	const TileSet* tileSet;
	dl_u16 vdpTileIndex;
	dl_u8 tilesPerFrame;
	dl_u8 vdpSize;
} GameSprite;

GameSprite playerSprite;
GameSprite dropsSprite;
GameSprite cursorSprite;
GameSprite ballSprite;
GameSprite birdSprite;
GameSprite keySprite;
GameSprite diamondSprite;
GameSprite moneyBagSprite;
GameSprite doorSprite;
GameSprite regenSprite;
GameSprite splatSprite;
GameSprite characterFontSprite;
GameSprite hudCharacterFontSprite;
GameSprite playerIconSprite;
GameSprite playerIconSpriteRegen;

const GameSprite* g_pickUpSprites[3];

#define REGEN_NUM_FRAMES 8 // regen frames per facing direction
dl_u8 g_regenSpriteIndex;

typedef void (*DrawRoomFunction)(struct GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(struct GameData* gameData, const Resources* resources);
void drawTitleScreen(struct GameData* gameData, const Resources* resources);
void drawTransition(struct GameData* gameData, const Resources* resources);
void drawWipeTransition(struct GameData* gameData, const Resources* resources);
void drawGetReadyScreen(struct GameData* gameData, const Resources* resources);

// character font
// player icons
// sound
// scrolling

//m_characterFont(resources->characterFont, 8, 7, 39),

typedef void (*RoomDrawFunction)(dl_u8 roomNumber, struct GameData* gameData, const Resources* resources);
RoomDrawFunction g_originalRoomDrawFunctions[NUM_ROOMS_AND_ALL];

dl_u16 g_numSpritesToDraw = 0;

dl_u16 g_vdpTileIndex = 0;
void buildTileResource(GameSprite* sprite, 
					   const TileSet* tileSet, 
					   dl_u8 spriteWidth, 
					   dl_u8 spriteHeight,
					   dl_u8 numFrames,
					   dl_u8 vdpSize)
{
	dl_u16 numTiles = ((spriteWidth + 7) / 8) * ((spriteHeight + 7) / 8);

	sprite->tileSet = tileSet;
	sprite->vdpTileIndex = g_vdpTileIndex;
	sprite->tilesPerFrame = numTiles;
	sprite->vdpSize = vdpSize;

	VDP_loadTileSet(tileSet, g_vdpTileIndex, DMA);

	g_vdpTileIndex += (numTiles * numFrames);
}

void buildSpriteResource(GameSprite* sprite, 
					     const SpriteDefinition* spriteDef, 
					     dl_u8 spriteWidth, 
					     dl_u8 spriteHeight,
					     dl_u8 numFrames,
					     dl_u8 vdpSize)
{
	dl_u16 numTiles = ((spriteWidth + 7) / 8) * ((spriteHeight + 7) / 8);

	sprite->tileSet = spriteDef->animations[0]->frames[0]->tileset;
	sprite->vdpTileIndex = g_vdpTileIndex;
	sprite->tilesPerFrame = numTiles;
	sprite->vdpSize = vdpSize;

	for (int loop = 0; loop < spriteDef->numAnimation; loop++)
	{
		TileSet* tileset = spriteDef->animations[loop]->frames[0]->tileset;

		VDP_loadTileSet(tileset, g_vdpTileIndex, DMA);

		g_vdpTileIndex += numTiles;
	}
}

/*
dl_u16 g_oamIndex = 0;

dl_u16 g_backgroundTileOffset;

dl_u16 buildSpriteResource(GameSprite* gameSprite,
						   const SpriteAttributes* spriteAttributes,
						   const dl_u8* sprite, 
						   dl_u8 width, 
						   dl_u8 height, 
						   dl_u8 spriteCount,
						   dl_u16 tileIndex)
{
	dl_u8 convertedSprite[384];
	memset(convertedSprite, 0, sizeof(convertedSprite));

	gameSprite->spriteAttributes = spriteAttributes;
	gameSprite->tileIndex = tileIndex;

	const dl_u8* spriteRunner = sprite;

	gameSprite->tilesPerFrame = ((width + 7) / 8) * ((height + 7) / 8);


	for (int loop = 0; loop < spriteCount; loop++)
    {

		convert1bppImageTo8bppCrtEffectImage(spriteRunner,
											 convertedSprite,
											 width,
											 height,
											 CrtColor_Blue);

		tileIndex += convertToTiles(convertedSprite, 
									width, 
									height, 
									CHAR_BASE_BLOCK(4),
									tileIndex * 64);

		spriteRunner += (width / 8) * height;
	}

	return tileIndex;
}

dl_u16 buildTextResource(GameSprite* gameSprite,
						 const SpriteAttributes* spriteAttributes,
						 const dl_u8* sprite, 
						 dl_u8 width, 
						 dl_u8 height, 
						 dl_u8 spriteCount,
						 dl_u16 tileIndex,
						 dl_u8 addDots)
{
	dl_u8 convertedSprite[64];
	memset(convertedSprite, 0, sizeof(convertedSprite));

	gameSprite->spriteAttributes = spriteAttributes;
	gameSprite->tileIndex = tileIndex;

	const dl_u8* spriteRunner = sprite;

	gameSprite->tilesPerFrame = ((width + 7) / 8) * ((height + 7) / 8);


	for (int loop = 0; loop < spriteCount; loop++)
    {

		convert1bppImageTo8bppCrtEffectImage(spriteRunner,
											 convertedSprite,
											 width,
											 height,
											 CrtColor_Blue);

		// convert the 0 pixel indexes to non-transparently black at index 4
		// convert the white pixels to blue
		for (int loop2 = 0; loop2 < 64; loop2++)
		{
			if (convertedSprite[loop2] == 0)
				convertedSprite[loop2] = 4;
			if (convertedSprite[loop2] == 3)
				convertedSprite[loop2] = 1;
		}

		if (addDots)
		{
			// add a dotted line at the bottom
			for (int loop2 = 56; loop2 < 64; loop2 += 2)
			{
				convertedSprite[loop2] = 4;
				convertedSprite[loop2 + 1] = 1;
			}
		}

		tileIndex += convertToTiles(convertedSprite, 
									width, 
									height, 
									CHAR_BASE_BLOCK(4),
									tileIndex * 64);

		spriteRunner += (width / 8) * height;
	}

	return tileIndex;
}

dl_u16 buildPlayerIconResource(GameSprite* gameSprite,
							   const SpriteAttributes* spriteAttributes,
							   const dl_u8* sprite, 
							   dl_u8 width, 
							   dl_u8 height, 
							   dl_u8 clipHeight,
							   dl_u8 spriteCount,
							   dl_u16 tileIndex)
{
	// use the player sprite but only the top part

	dl_u8 convertedSprite[256];
	memset(convertedSprite, 0, sizeof(convertedSprite));

	gameSprite->spriteAttributes = spriteAttributes;
	gameSprite->tileIndex = tileIndex;

	const dl_u8* spriteRunner = sprite;

	gameSprite->tilesPerFrame = ((width + 7) / 8) * ((clipHeight + 7) / 8);


	for (int loop = 0; loop < spriteCount; loop++)
    {

		convert1bppImageTo8bppCrtEffectImage(spriteRunner,
											 convertedSprite,
											 width,
											 height,
											 CrtColor_Blue);

		// convert the 0 pixel indexes to non-transparently black at index 4
		for (int loop2 = 0; loop2 < 256; loop2++)
		{
			if (convertedSprite[loop2] == 0)
				convertedSprite[loop2] = 4;
		}

		// add a dotted line at the bottom
		for (int loop2 = 112; loop2 < 128; loop2 += 2)
		{
			convertedSprite[loop2] = 4;
			convertedSprite[loop2 + 1] = 1;
		}

		tileIndex += convertToTiles(convertedSprite, 
									width, 
									clipHeight, 
									CHAR_BASE_BLOCK(4),
									tileIndex * 64);

		spriteRunner += (width / 8) * height;
	}

	return tileIndex;
}

dl_u16 buildEmptySpriteResource(GameSprite* gameSprite,
								const SpriteAttributes* spriteAttributes,
								dl_u8 width, 
								dl_u8 height, 
								dl_u8 spriteCount,
								dl_u16 tileIndex)
{
	gameSprite->spriteAttributes = spriteAttributes;
	gameSprite->tileIndex = tileIndex;
	gameSprite->tilesPerFrame = ((width + 7) / 8) * ((height + 7) / 8);

	tileIndex += gameSprite->tilesPerFrame;

	return tileIndex;
}

void updateRegenSprite(const Resources* resources, dl_u8 currentPlayerSpriteNumber)
{
	const dl_u16 bufferSize = (PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS;
	dl_u8 convertedSprite[384];
	dl_u8 regenBuffer[bufferSize];
    memset(regenBuffer, 0, bufferSize);

    const dl_u8* originalSprite = resources->sprites_player;
    originalSprite += currentPlayerSpriteNumber * bufferSize;

    drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													PLAYER_SPRITE_ROWS,
													regenBuffer);

	convert1bppImageTo8bppCrtEffectImage(regenBuffer,
										 convertedSprite,
										 PLAYER_SPRITE_WIDTH,
										 PLAYER_SPRITE_ROWS,
										 CrtColor_Blue);

	convertToTiles(convertedSprite, 
				   PLAYER_SPRITE_WIDTH, 
				   PLAYER_SPRITE_ROWS, 
				   CHAR_BASE_BLOCK(4),
				   regenSprite.tileIndex * 64);
}

void buildUI()
{
	// build the background for score and lives
	dl_u16* vramTileAddr = (dl_u16*)VRAM;

	// copy empty tile
	dl_u8 tile[32];
	memset(tile, 0x0, sizeof(tile));
	CpuFastSet(tile, vramTileAddr, COPY32 | 8);

	// build and copy UI tile
	vramTileAddr += 16;

	memset(tile, 0x44, sizeof(tile));
	for (int loop = 28; loop < 32; loop++)
		tile[loop] = 0x14;

	CpuFastSet(tile, vramTileAddr, COPY32 | 8);
	// build and copy UI tile
	vramTileAddr += 16;


	// build tilemap for UI bar
	dl_u16* vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(UI_TILEMAP_INDEX);
	for (int loop = 0; loop < 30; loop++)
	{
		*vramTileMapAddr = 1;
		vramTileMapAddr++;
	}

	// build tile for transition background
    // first line: transparent
    // second line: non-transparent black
    // third line: blue
	dl_u8 transitionTile[64];
    for (int i = 0; i < 8; i++)
    {
        transitionTile[i] = 0x00;
        transitionTile[i + 8] = 0x04; // non transparent black
        transitionTile[i + 16] = 0x01; // blue
    }

	CpuFastSet(transitionTile, vramTileAddr, COPY32 | 16);
	vramTileAddr += 32;

	// build tilemap for UI bar
	vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(TRANSITION_TILEMAP_INDEX);
	for (int loop = 0; loop < 16; loop++)
	{
		*vramTileMapAddr = 0x0202;
		*(vramTileMapAddr + 16) = 0x0101;
		vramTileMapAddr++;
	}

	// build tilemap for the transition background
    vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(TRANSITION_TILEMAP_INDEX);
    // fill only the first line of bg2 with the transition tile. 
    for (int i = 0; i < 16 ; i++) 
    {
        vramTileMapAddr[i] = 0x0101;
    }

	g_backgroundTileOffset = 6;
}
*/

void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

/*
void createBackgrounds(const GameData* gameData, 
					   const Resources* resources)
{
	// change which tilemap to use

	dl_u16* vramTileAddr = (dl_u16*)VRAM;

	dl_u16 tileOffset = g_backgroundTileOffset;

	for (int loop = 0; loop < NUM_ROOMS_AND_ALL; loop++)
	{
		if (g_rooms[loop]->draw == NULL)
			continue;

		dl_u16* vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(GAME_TILEMAP_INDEX - loop);

		g_rooms[loop]->draw(loop, (GameData*)gameData, resources);

		tileOffset = convertBackgroundToVRAM16(gameData->cleanBackground,
											   vramTileAddr,
											   vramTileMapAddr,
											   tileOffset,
											   FRAMEBUFFER_WIDTH,
											   FRAMEBUFFER_HEIGHT,
											   CrtColor_Blue);
	}

	// we needed an active GameData to render rooms
	// but we screwed up the current room. Force it 
	// back to the current room when the game started.
	dl_u8 roomNumber = gameData->transitionRoomNumber;
	g_rooms[roomNumber]->draw(roomNumber, (GameData*)gameData, resources);
}

void setGameBackgroundTilemap(dl_u8 tilemapIndex)
{
	BGCTRL[GAME_BACKGROUND_INDEX] = SCREEN_BASE(GAME_TILEMAP_INDEX - tilemapIndex) |
									BG_16_COLOR |
									BG_PRIORITY(GAME_BACKGROUND_PRIORITY) |
									BG_SIZE_0;	
}

void buildSplatSpriteResource(dl_u16 tileIndex, const Resources* resources)
{
	dl_u16 newWidth = 32;
	dl_u8 convertedSprite[512];
	memset(convertedSprite, 0, sizeof(convertedSprite));

	splatSprite.spriteAttributes = &g_32x16SpriteAttributes;
	splatSprite.tileIndex = tileIndex;
	splatSprite.tilesPerFrame = 4 * 2;

	convert1bppImageTo8bppCrtEffectImageWithNewWidth(resources->sprite_playerSplat,
													 convertedSprite,
													 PLAYER_SPLAT_SPRITE_WIDTH,
													 PLAYER_SPLAT_SPRITE_ROWS,
													 newWidth,
													 CrtColor_Blue);

	tileIndex += convertToTiles(convertedSprite, 
								newWidth, 
								PLAYER_SPLAT_SPRITE_ROWS, 
								CHAR_BASE_BLOCK(4),
								tileIndex * 64);

	convertToTiles(convertedSprite, 
				   newWidth, 
				   PLAYER_SPLAT_SPRITE_ROWS, 
				   CHAR_BASE_BLOCK(4),
				   tileIndex * 64);

	// erase the first five rows of the second set of splat tiles
	dl_u16* vramAddress = (CHAR_BASE_BLOCK(4) + (tileIndex * 64));
	for (int loop = 0; loop < 5 * 4; loop++)
	{
		vramAddress[loop] = 0;
		vramAddress[loop + 32] = 0;
		vramAddress[loop + 64] = 0;
	}
}

void drawCustomTitleScreenBackground(GameData* gameData, const Resources* resources)
{
	dl_u8* framebuffer = gameData->cleanBackground;

	// draw custom background for titlescreen on GBA.
	const dl_u8 numCommands = 6;
	BackgroundDrawCommand backgroundDrawCommands[numCommands];
	memcpy(backgroundDrawCommands, 
		   resources->roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData.backgroundDrawCommands, 
		   sizeof(BackgroundDrawCommand) * numCommands);

	backgroundDrawCommands[2].drawCount = 9;
	backgroundDrawCommands[4].drawCount = 9;

	BackgroundDrawData backgroundDrawData;
	backgroundDrawData.drawCommandCount = numCommands;
	backgroundDrawData.backgroundDrawCommands = backgroundDrawCommands;

	drawBackground(&backgroundDrawData, 
				   resources,
				   framebuffer);
}
*/

void drawTileText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 tilex = ((xyLocation % 32) * 8) / 8;
    dl_u16 tiley = (xyLocation / 32) / 8;

    // for each character
    while (*text != 0xff)
    {
		//drawSprite(x, y, *text, font);
		VDP_setTileMapXY(BG_A, characterFontSprite.vdpTileIndex + *text, tilex, tiley);

        text++;
        tilex++;
    }
}

void custom_chamber_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	VDP_setTileMapDataRect(BG_A, roomTileMaps[roomNumber], 0, 0, TILE_MAP_WIDTH, TILE_MAP_HEIGHT, TILE_MAP_WIDTH, DMA);

	// init background and text
	g_originalRoomDrawFunctions[roomNumber](roomNumber, gameData, resources);
}

void custom_titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	VDP_setTileMapDataRect(BG_A, roomTileMaps[roomNumber], 0, 0, TILE_MAP_WIDTH, TILE_MAP_HEIGHT, TILE_MAP_WIDTH, DMA);

	/*
	drawCustomTitleScreenBackground(gameData, resources);

	dl_u8* framebuffer = gameData->cleanBackground;

	// title screen text
	drawText(resources->text_downland, resources->characterFont, framebuffer, 0x03c9); // 0x07c9 original coco mem location
	drawText(resources->text_writtenBy + 8, resources->characterFont, framebuffer, 0x050a - 5); // 0x090A original coco mem location
	drawText(resources->text_michaelAichlmayer, resources->characterFont, framebuffer, 0x647 + 1 - (0x28 * 8)); // 0x0A47 original coco mem location
	drawText(resources->text_copyright1983, resources->characterFont, framebuffer, 0x789 - (0x20 * 8)); // 0x0B89 original coco mem location
	drawText(resources->text_spectralAssociates, resources->characterFont, framebuffer, 0x8c6 - (0x20 * 8)); // 0x0CC6 original coco mem location
	drawText(resources->text_licensedTo, resources->characterFont, framebuffer, 0xa0a - (0x20 * 8)); // 0x0E0A original coco mem location
	drawText(resources->text_tandyCorporation, resources->characterFont, framebuffer, 0xb47 - (0x20 * 8)); // 0x0F47 original coco mem location
	drawText(resources->text_allRightsReserved, resources->characterFont, framebuffer, 0xc86 - (0x20 * 8)); // 0x1086 original coco mem location
	drawText(resources->text_onePlayer, resources->characterFont, framebuffer, 0xf05 - (0x20 * 8)); // 0x1305 original coco mem location
	drawText(resources->text_twoPlayer, resources->characterFont, framebuffer, 0xf11 - (0x20 * 8)); // 0x1311 original coco mem location
	drawText(resources->text_highScore, resources->characterFont, framebuffer, 0x118b - 5 - (0x20 * 8)); // 0x158B original coco mem location
	drawText(resources->text_playerOne, resources->characterFont, framebuffer, 0x1406 - (0x20 * 17)); // 0x1806 original coco mem location
	drawText(resources->text_playerTwo, resources->characterFont, framebuffer, 0x1546 - (0x20 * 16)); // 0x1946 original coco mem location
	*/

	// init background and text
//	drawBackground(&resources->roomResources[roomNumber].backgroundDrawData, 
//				   resources,
//				   gameData->cleanBackground);

	g_originalRoomDrawFunctions[roomNumber](roomNumber, gameData, resources);

	// title screen text
	drawTileText(resources->text_downland, 0x03c9 + 64); // 0x07c9 original coco mem location
	drawTileText(resources->text_writtenBy, 0x050a); // 0x090A original coco mem location
	drawTileText(resources->text_michaelAichlmayer, 0x647); // 0x0A47 original coco mem location
	drawTileText(resources->text_copyright1983, 0x789); // 0x0B89 original coco mem location
	drawTileText(resources->text_spectralAssociates, 0x8c6); // 0x0CC6 original coco mem location
	drawTileText(resources->text_licensedTo, 0xa0a); // 0x0E0A original coco mem location
	drawTileText(resources->text_tandyCorporation, 0xb47); // 0x0F47 original coco mem location
	drawTileText(resources->text_allRightsReserved, 0xc86); // 0x1086 original coco mem location
	drawTileText(resources->text_onePlayer, 0xf05); // 0x1305 original coco mem location
	drawTileText(resources->text_twoPlayer, 0xf11); // 0x1311 original coco mem location
	drawTileText(resources->text_highScore, 0x118b); // 0x158B original coco mem location
	drawTileText(resources->text_playerOne, 0x1406); // 0x1806 original coco mem location
	drawTileText(resources->text_playerTwo, 0x1546); // 0x1946 original coco mem location

	convertScoreToString(gameData->playerData[PLAYER_ONE].score, gameData->playerData[PLAYER_ONE].scoreString);
	drawTileText(gameData->playerData[PLAYER_ONE].scoreString, TITLESCREEN_PLAYERONE_SCORE_LOCATION);

	convertScoreToString(gameData->playerData[PLAYER_TWO].score, gameData->playerData[PLAYER_TWO].scoreString);
	drawTileText(gameData->playerData[PLAYER_TWO].scoreString, TITLESCREEN_PLAYERTWO_SCORE_LOCATION);

	if (gameData->playerData[PLAYER_ONE].score > gameData->highScore)
		gameData->highScore = gameData->playerData[PLAYER_ONE].score;
	else if (gameData->playerData[PLAYER_TWO].score > gameData->highScore)
		gameData->highScore = gameData->playerData[PLAYER_TWO].score;

	convertScoreToString(gameData->highScore, gameData->string_highScore);

	drawTileText(gameData->string_highScore, TITLESCREEN_HIGHSCORE_LOCATION);
}

void custom_get_ready_room_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	VDP_setTileMapDataRect(BG_A, roomTileMaps[TITLESCREEN_ROOM_INDEX], 0, 0, TILE_MAP_WIDTH, TILE_MAP_HEIGHT, TILE_MAP_WIDTH, DMA);

	// init background and text
	g_originalRoomDrawFunctions[roomNumber](roomNumber, gameData, resources);

	// get ready text
	const dl_u8* getReadyString = gameData->currentPlayerData->playerNumber == PLAYER_ONE ? resources->text_getReadyPlayerOne : resources->text_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);
}

void GameRunner_Init(GameData* gameData, const Resources* resources)
{
	/*
	// setup sprite attributes
	g_8x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_8x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_8x8SpriteAttributes.attr2 = OBJ_PRIORITY(SPRITE_PRIORITY);

	g_16x16SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_16x16SpriteAttributes.attr1 = ATTR1_SIZE_16;
	g_16x16SpriteAttributes.attr2 = OBJ_PRIORITY(SPRITE_PRIORITY);

	g_32x16SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_WIDE;
	g_32x16SpriteAttributes.attr1 = ATTR1_SIZE_32;
	g_32x16SpriteAttributes.attr2 = OBJ_PRIORITY(SPRITE_PRIORITY);

	g_16x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_WIDE;
	g_16x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_16x8SpriteAttributes.attr2 = OBJ_PRIORITY(SPRITE_PRIORITY);

	g_textSpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_textSpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_textSpriteAttributes.attr2 = OBJ_PRIORITY(SPRITE_PRIORITY);

	g_hudTextSpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_hudTextSpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_hudTextSpriteAttributes.attr2 = OBJ_PRIORITY(UI_SPRITE_PRIORITY);

	g_playerIconSpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_WIDE;
	g_playerIconSpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_playerIconSpriteAttributes.attr2 = OBJ_PRIORITY(UI_SPRITE_PRIORITY);

	dl_u32 cursorSpriteRaw = 0xffffffff;
	*/

	// load background
	VDP_loadTileSet(&backgroundTileset, 0, DMA);
	g_vdpTileIndex = backgroundTileset.numTile;


	// load sprite tile resources
	buildTileResource(&dropsSprite, &dropTileset, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT, SPRITE_SIZE(1, 1));
	buildSpriteResource(&playerSprite, &playerTileset, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT, SPRITE_SIZE(2, 2));
	buildSpriteResource(&cursorSprite, &cursorTileset, 8, 1, 1, SPRITE_SIZE(1,1));
	buildTileResource(&ballSprite, &ballTileset, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT, SPRITE_SIZE(2, 1));
	buildTileResource(&birdSprite, &birdTileset, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT, SPRITE_SIZE(2, 1));
	buildSpriteResource(&keySprite, &keyTileset, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, SPRITE_SIZE(2, 2));
	buildSpriteResource(&diamondSprite, &diamondTileset, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, SPRITE_SIZE(2, 2));
	buildSpriteResource(&moneyBagSprite, &moneyBagTileset, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, SPRITE_SIZE(2, 2));
	buildSpriteResource(&doorSprite, &doorTileset, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1, SPRITE_SIZE(2, 2));
	buildSpriteResource(&regenSprite, &regenTileset, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 10, SPRITE_SIZE(2,2));	
	buildSpriteResource(&splatSprite, &playerSplatTileset, 24, 16, 2, SPRITE_SIZE(3,2));	
	buildTileResource(&characterFontSprite, &characterFontTileset, 8, 8, CHARACTER_FONT_COUNT, SPRITE_SIZE(1, 1));

	//tileIndex = buildPlayerIconResource(&playerIconSprite, &g_playerIconSpriteAttributes, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, tileIndex);

	/*
	buildSplatSpriteResource(tileIndex, resources);

	// create the regen sprite for player icon
	playerIconSpriteRegen.spriteAttributes = &g_playerIconSpriteAttributes;
	playerIconSpriteRegen.tileIndex = regenSprite.tileIndex;
	playerIconSpriteRegen.tilesPerFrame = regenSprite.tilesPerFrame;
	*/

	g_pickUpSprites[0] = &diamondSprite;
	g_pickUpSprites[1] = &moneyBagSprite;
	g_pickUpSprites[2] = &keySprite;

	/*
	g_scrollX = 7;
	g_scrollY = 13;
	*/

	// room draw setup
    m_drawRoomFunctions[0] = drawChamber;
    m_drawRoomFunctions[1] = drawChamber;
    m_drawRoomFunctions[2] = drawChamber;
    m_drawRoomFunctions[3] = drawChamber;
    m_drawRoomFunctions[4] = drawChamber;
    m_drawRoomFunctions[5] = drawChamber;
    m_drawRoomFunctions[6] = drawChamber;
    m_drawRoomFunctions[7] = drawChamber;
    m_drawRoomFunctions[8] = drawChamber;
    m_drawRoomFunctions[9] = drawChamber;
    m_drawRoomFunctions[TITLESCREEN_ROOM_INDEX] = drawTitleScreen;
    m_drawRoomFunctions[TRANSITION_ROOM_INDEX] = drawTransition;
    m_drawRoomFunctions[WIPE_TRANSITION_ROOM_INDEX] = drawWipeTransition;
    m_drawRoomFunctions[GET_READY_ROOM_INDEX] = drawGetReadyScreen;

	//buildUI();


	g_originalRoomDrawFunctions[0] = g_rooms[0]->draw;
	g_originalRoomDrawFunctions[1] = g_rooms[1]->draw;
	g_originalRoomDrawFunctions[2] = g_rooms[2]->draw;
	g_originalRoomDrawFunctions[3] = g_rooms[3]->draw;
	g_originalRoomDrawFunctions[4] = g_rooms[4]->draw;
	g_originalRoomDrawFunctions[5] = g_rooms[5]->draw;
	g_originalRoomDrawFunctions[6] = g_rooms[6]->draw;
	g_originalRoomDrawFunctions[7] = g_rooms[7]->draw;
	g_originalRoomDrawFunctions[8] = g_rooms[8]->draw;
	g_originalRoomDrawFunctions[9] = g_rooms[9]->draw;
	g_originalRoomDrawFunctions[TITLESCREEN_ROOM_INDEX] = g_rooms[TITLESCREEN_ROOM_INDEX]->draw;
	g_originalRoomDrawFunctions[GET_READY_ROOM_INDEX] = g_rooms[GET_READY_ROOM_INDEX]->draw;


    g_rooms[0]->draw = custom_chamber_draw;
    g_rooms[1]->draw = custom_chamber_draw;
    g_rooms[2]->draw = custom_chamber_draw;
    g_rooms[3]->draw = custom_chamber_draw;
    g_rooms[4]->draw = custom_chamber_draw;
    g_rooms[5]->draw = custom_chamber_draw;
    g_rooms[6]->draw = custom_chamber_draw;
    g_rooms[7]->draw = custom_chamber_draw;
    g_rooms[8]->draw = custom_chamber_draw;
    g_rooms[9]->draw = custom_chamber_draw;
	g_rooms[TITLESCREEN_ROOM_INDEX]->draw = custom_titleScreen_draw;
	g_rooms[GET_READY_ROOM_INDEX]->draw = custom_get_ready_room_draw;


	//Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;

	//setGameBackgroundTilemap(TITLESCREEN_ROOM_INDEX);

	Game_Init(gameData, resources);

	//createBackgrounds(gameData, resources);

}

void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType)
{
	/*
	g_oldPlayerX = 0xffff;
	g_oldPlayerY = 0xffff;

	dl_u32 mode = MODE_1 | BG1_ON | BG2_ON | OBJ_ENABLE | OBJ_1D_MAP;

	if (roomNumber == WIPE_TRANSITION_ROOM_INDEX || 
		roomNumber == TRANSITION_ROOM_INDEX)
	{
		g_transitionCounter = TRANSITION_BLACK_SCREEN;
	}
	else
	{
		g_transitionCounter = TRANSITION_OFF;
	}

	if (roomNumber != TITLESCREEN_ROOM_INDEX &&
		roomNumber != GET_READY_ROOM_INDEX)
	{
		// turn on the UI hud
		mode |= BG0_ON;
	}
	else
	{
		g_scrollX = 7;
		g_scrollY = 13;
	}

	SetMode(mode);
	*/
}

void GameRunner_Update(GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

//void drawUIText(const dl_u8* text, dl_u16 x, dl_u16 y, GameSprite* font);

void GameRunner_Draw(GameData* gameData, const Resources* resources)
{
	g_numSpritesToDraw = 0;
	VDP_clearSprites();

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);

	VDP_setSpriteLink(g_numSpritesToDraw - 1, 0);
	VDP_updateSprites(g_numSpritesToDraw, DMA);

	//KDebug_AlertNumber(g_numSpritesToDraw);
}

// draw sprite, affected by scrolling
void drawSprite(dl_u16 x, dl_u16 y, dl_u8 frame, const GameSprite* gameSprite)
{
	dl_u16 tileIndex = gameSprite->vdpTileIndex + (frame * gameSprite->tilesPerFrame);

	VDP_setSpriteFull(g_numSpritesToDraw, x, y, gameSprite->vdpSize, tileIndex, g_numSpritesToDraw + 1);
	g_numSpritesToDraw++;
}

void drawDrops(const GameData* gameData)
{
    // draw drops
    const Drop* dropsRunner = gameData->dropData.drops;

    for (int loop = 0; loop < NUM_DROPS; loop++)
    {
        if ((dl_s8)dropsRunner->wiggleTimer < 0 || // wiggling
            dropsRunner->wiggleTimer > 1)   // falling
        {
			drawSprite((dropsRunner->x << 1), 
					   (dropsRunner->y >> 8), 
					   0,
					   &dropsSprite);
        }

        dropsRunner++;
    }
}

void drawUIText(const dl_u8* text, dl_u16 x, dl_u16 y, GameSprite* font)
{
    // for each character
    while (*text != 0xff)
    {
		drawSprite(x, y, *text, font);

        text++;
        x += 8;
    }
}

void drawUIPlayerLives(const PlayerData* playerData)
{
	/*
	dl_u8 x = 80;//PLAYERLIVES_ICON_X;
	dl_u8 y = 0;//PLAYERLIVES_ICON_Y;

    for (dl_u8 loop = 0; loop < playerData->lives; loop++)
	{
        drawSprite(x, 
				   y, 
				   playerData->currentSpriteNumber,
				   &playerIconSprite);

		x += 24;//PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
        drawSprite(x, 
                   y, 
				   0,
				   &playerIconSpriteRegen);		
    }
	*/
}

/*
void updateScroll(dl_u16 playerX, dl_u16 playerY)
{
	if (g_oldPlayerX == 0xffff)
	{
		g_oldPlayerX = playerX;
		g_oldPlayerY = playerY;

		g_scrollX = playerX > 128 ? SCROLL_MAX_X : 0;
		g_scrollY = playerY > 96 ? SCROLL_MAX_Y : 0;
	}

	dl_u16 screenX = playerX - g_scrollX;
	dl_u16 screenY = playerY - g_scrollY;
	
	dl_s16 deltaX = (dl_s16)playerX - (dl_s16)g_oldPlayerX;
	dl_s16 deltaY = (dl_s16)playerY - (dl_s16)g_oldPlayerY;
	
	if (deltaX < 0 && screenX < 100)
	{
		g_scrollX += deltaX;
	}
	
	if (deltaX > 0 && screenX > 140)
	{
		g_scrollX += deltaX;
	}

	if (deltaY < 0 && screenY < 60)
	{
		g_scrollY += deltaY;
	}
	
	if (deltaY > 0 && screenY > 100)
	{
		g_scrollY += deltaY;
	}
	
	if (g_scrollX < 0)
		g_scrollX = 0;
	if (g_scrollX > SCROLL_MAX_X)
		g_scrollX = SCROLL_MAX_X;

	if (g_scrollY < 0)
		g_scrollY = 0;
	if (g_scrollY > SCROLL_MAX_Y)
		g_scrollY = SCROLL_MAX_Y;
	
	g_oldPlayerX = playerX;
	g_oldPlayerY = playerY;
}
*/

void drawChamber(struct GameData* gameData, const Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;

	dl_u16 playerX = (playerData->x >> 8) << 1;
	dl_u16 playerY = (playerData->y >> 8);

	//updateScroll(playerX, playerY);

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];


	drawDrops(gameData);

	
    // draw ball
    if (gameData->ballData.enabled)
    {
        const BallData* ballData = &gameData->ballData;

		drawSprite((ballData->x >> 8) << 1,
				   ballData->y >> 8,
				   (dl_s8)ballData->fallStateCounter < 0,
				   &ballSprite);
    }

    // draw bird
    if (gameData->birdData.state && currentTimer == 0)
    {
        const BirdData* birdData = &gameData->birdData;

		drawSprite((birdData->x >> 8) << 1,
				   birdData->y >> 8,
				   birdData->animationFrame,
				   &birdSprite);
    }

	// draw player
    switch (playerData->state)
    {
    case PLAYER_STATE_SPLAT: 
        drawSprite(playerX,
                   playerY + 7,
                   playerData->splatFrameNumber,
				   &splatSprite);
	
        break;
    case PLAYER_STATE_REGENERATION: 
	
        if (!gameData->paused)
        {
            g_regenSpriteIndex++;
			if (g_regenSpriteIndex == REGEN_NUM_FRAMES - 1)
				g_regenSpriteIndex = 0;
        }

        drawSprite(playerX,
                   playerY,
                   g_regenSpriteIndex + (playerData->facingDirection ? 0 : REGEN_NUM_FRAMES),
				   &regenSprite);
	
        break;
	
    default: 
        drawSprite(playerX,
                   playerY,
                   playerData->currentSpriteNumber,
				   &playerSprite);
    }


	// draw pickups
    const Pickup* pickups = playerData->gamePickups[gameData->currentRoom->roomNumber];
    for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
    {
		if ((pickups->state & playerData->playerMask))
		{
			drawSprite(pickups->x << 1,
					   pickups->y,
					   0,
					   g_pickUpSprites[pickups->type]);
        }

        pickups++;
    }

	// draw doors
    int roomNumber = gameData->currentRoom->roomNumber;
	const DoorInfoData* doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
	const DoorInfo* doorInfoRunner = doorInfoData->doorInfos;

	for (dl_u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
        if (playerData->doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask &&
			doorInfoRunner->x != 0xff)
		{
			int xPosition = doorInfoRunner->x;
			// adjust the door position, as per the original game.
			if (xPosition > 40) 
				xPosition += 7;
			else
				xPosition -= 4;

			drawSprite(xPosition << 1,
					   doorInfoRunner->y,
					   0,
					   &doorSprite);
		}

		doorInfoRunner++;
	}

    // draw text
	//drawUIText(gameData->string_timer, 24 * 8, 0, &hudCharacterFont);
	//drawUIText(playerData->scoreString, 8, 0, &hudCharacterFont); 
	//
	//drawUIPlayerLives(playerData);
}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);


	drawSprite(gameData->numPlayers == 1 ? 32 : 128,
			   123,
			   0,
			   &cursorSprite);

	/*
	convertScoreToString(gameData->playerData[PLAYER_ONE].score, gameData->playerData[PLAYER_ONE].scoreString);
	convertScoreToString(gameData->playerData[PLAYER_TWO].score, gameData->playerData[PLAYER_TWO].scoreString);

	if (gameData->playerData[PLAYER_ONE].score > gameData->highScore)
		gameData->highScore = gameData->playerData[PLAYER_ONE].score;
	else if (gameData->playerData[PLAYER_TWO].score > gameData->highScore)
		gameData->highScore = gameData->playerData[PLAYER_TWO].score;

	convertScoreToString(gameData->highScore, gameData->string_highScore);

	drawUIText(gameData->string_highScore, 136, 118, &characterFont);
	drawUIText(gameData->playerData[PLAYER_ONE].scoreString, 136, 129, &characterFont);
	drawUIText(gameData->playerData[PLAYER_TWO].scoreString, 136, 140, &characterFont);
	*/
}

void clearBackground()
{

}

void drawCleanBackground(const GameData* gameData, 
						 const Resources* resources,
						 dl_u8* cleanBackground, 
						 dl_u16 tileOffset)
{
	// change which tilemap to use
	//setGameBackgroundTilemap(gameData->transitionRoomNumber);
}

void drawTransition(struct GameData* gameData, const Resources* resources)
{
	/*
	if (gameData->transitionInitialDelay == 29)
    {
        g_transitionCounter = TRANSITION_BLACK_SCREEN;
    }
    else if (!gameData->transitionInitialDelay)
    {
		g_transitionCounter = TRANSITION_OFF;
        drawCleanBackground(gameData, 
							resources, 
							gameData->cleanBackground, 
							g_backgroundTileOffset);
    }
	*/
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{
	/*
	if (gameData->transitionInitialDelay == 29)
    {
        g_transitionCounter = TRANSITION_BLACK_SCREEN;

		PlayerData* playerData = gameData->currentPlayerData;

		dl_u16 playerX;
		dl_u16 playerY;

		if (playerData->lastDoor)
		{
			playerX = playerData->lastDoor->xLocationInNextRoom;
			playerY = playerData->lastDoor->yLocationInNextRoom;
		}
		else
		{
			playerX = PLAYER_START_X;
			playerY = PLAYER_START_Y;
		}

		updateScroll(playerX << 1, playerY);
    }
    else if (!gameData->transitionInitialDelay)
    {
		//g_transitionCounter = TRANSITION_OFF;
        drawCleanBackground(gameData, 
							resources, 
							gameData->cleanBackground, 
							g_backgroundTileOffset);
    }

	if (!gameData->transitionCurrentLine)
		g_transitionCounter = TRANSITION_BLACK_SCREEN;
	else
		g_transitionCounter = gameData->transitionCurrentLine;
		*/
}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
