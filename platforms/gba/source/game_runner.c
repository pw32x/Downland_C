#include "game_runner.h"

#include <gba_sprites.h>
#include <gba_video.h>

#include "image_utils.h"

#include "..\..\..\game\drops_manager.h"

typedef struct
{
	dl_u16 attr0;
	dl_u16 attr1;
	dl_u16 attr2;
} SpriteAttributes;

SpriteAttributes g_8x8SpriteAttributes;
SpriteAttributes g_16x16SpriteAttributes;

int playerSpriteTileIndex;
int dropSpriteTileIndex;

typedef void (*DrawRoomFunction)(struct GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(struct GameData* gameData, const Resources* resources);
void drawTitleScreen(struct GameData* gameData, const Resources* resources);
void drawTransition(struct GameData* gameData, const Resources* resources);
void drawWipeTransition(struct GameData* gameData, const Resources* resources);
void drawGetReadyScreen(struct GameData* gameData, const Resources* resources);

//m_dropSprite(resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT),
//m_ballSprite(resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT),
//m_playerSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT),
//m_playerSplatSprite(resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_COUNT),
//m_birdSprite(resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT),
//m_keySprite(resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
//m_diamondSprite(resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
//m_moneyBagSprite(resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1),
//m_cursorSprite(&m_cursor1bppSprite, 8, 1, 1),
//m_regenSprite(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_ROWS, REGEN_SPRITES),
//m_characterFont(resources->characterFont, 8, 7, 39),

dl_u16 loadResourceToTiles(const dl_u8* sprite, dl_u8 width, dl_u8 height, dl_u16 tileIndex)
{
	dl_u8 convertedSprite[256];

	convert1bppImageTo8bppCrtEffectImage(sprite,
                                         convertedSprite,
                                         width,
                                         height,
                                         CrtColor_Blue);

	dl_u16 tileCount = convertToTiles(convertedSprite, 
									  width, 
									  height, 
									  CHAR_BASE_BLOCK(4),
									  tileIndex * 32); // 32 bytes after the last sprite

	return tileIndex + tileCount;
}

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	// load tile resources
	dropSpriteTileIndex = 0;
	playerSpriteTileIndex = loadResourceToTiles(resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, dropSpriteTileIndex);
	loadResourceToTiles(resources->sprites_player, 16, 16, playerSpriteTileIndex);

	g_8x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_8x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_8x8SpriteAttributes.attr2 = 0;

	g_16x16SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_16x16SpriteAttributes.attr1 = ATTR1_SIZE_16;
	g_16x16SpriteAttributes.attr2 = 0;

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

	//extern Room transitionRoom;
	//g_rooms[WIPE_TRANSITION_ROOM_INDEX] = &transitionRoom;

	Game_Init(gameData, resources);
}

void GameRunner_Update(struct GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(struct GameData* gameData, const Resources* resources)
{
	 m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);
}

dl_u16 drawDrops(const GameData* gameData, dl_u16 oamIndex)
{
    // draw drops
    const Drop* dropsRunner = gameData->dropData.drops;

    for (int loop = 0; loop < NUM_DROPS; loop++)
    {
        if ((dl_s8)dropsRunner->wiggleTimer < 0 || // wiggling
            dropsRunner->wiggleTimer > 1)   // falling
        {
			OAM[oamIndex].attr0 = g_8x8SpriteAttributes.attr0 | ((dropsRunner->y >> 8) & 0xff);
			OAM[oamIndex].attr1 = g_8x8SpriteAttributes.attr1 | ((dropsRunner->x << 1) & 0x1ff);
			OAM[oamIndex].attr2 = g_8x8SpriteAttributes.attr2 | dropSpriteTileIndex;

			oamIndex++;
        }

        dropsRunner++;
    }

	return oamIndex;
}

void drawChamber(struct GameData* gameData, const Resources* resources)
{

}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	dl_u16 oamIndex = 0;
	oamIndex = drawDrops(gameData, oamIndex);

	// draw
	dl_u16 x;
	dl_u16 y;
	dl_u16 tileIndex;

	// drop
	x = 120;
	y = 80;
	tileIndex = 0;//dropSpriteTileIndex;

	OAM[oamIndex].attr0 = g_8x8SpriteAttributes.attr0 | (y & 0xff);
	OAM[oamIndex].attr1 = g_8x8SpriteAttributes.attr1 | (x & 0x1ff);
	OAM[oamIndex].attr2 = g_8x8SpriteAttributes.attr2 | tileIndex;

	// player
	oamIndex++;
	x = 60;
	y = 40;
	tileIndex = playerSpriteTileIndex;

	OAM[oamIndex].attr0 = g_16x16SpriteAttributes.attr0 | (y & 0xff);
	OAM[oamIndex].attr1 = g_16x16SpriteAttributes.attr1 | (x & 0x1ff);
	OAM[oamIndex].attr2 = g_16x16SpriteAttributes.attr2 | tileIndex;

	for (int i = oamIndex + 1; i < 128; i++) 
	{
		OAM[i].attr0 = ATTR0_DISABLED;
		OAM[i].attr1 = 0;
		OAM[i].attr2 = 0;
	}
}

void clearBackground()
{

}



void drawCleanBackground(dl_u8* cleanBackground)
{
	dl_u16* vram = (dl_u16*)VRAM;

	//static dl_u8 convertedBackground[FRAMEBUFFER_WIDTH * 8];

	convert1bppImageToVRAMCrtEffectImage(cleanBackground,
                                         vram,
                                         FRAMEBUFFER_WIDTH,
                                         FRAMEBUFFER_HEIGHT,
                                         CrtColor_Blue);

	/*

	for (int loop = 0; loop < FRAMEBUFFER_WIDTH * 8; loop++)
		convertedBackground[loop] = 1;

	convertToTiles(convertedBackground, 
				   FRAMEBUFFER_WIDTH, 
				   8, 
				   CHAR_BASE_BLOCK(4),
				   0);
	*/
}

void drawTransition(struct GameData* gameData, const Resources* resources)
{
	if (gameData->transitionInitialDelay == 29)
    {
        clearBackground();
    }
    else if (!gameData->transitionInitialDelay)
    {
        drawCleanBackground(gameData->cleanBackground);
    }
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{

}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{

}
