#include "game_runner.h"

#include <gba_sprites.h>
#include <gba_video.h>

#include <string.h>

#include "image_utils.h"

#include "..\..\..\game\drops_manager.h"

typedef struct
{
	dl_u16 attr0;
	dl_u16 attr1;
	dl_u16 attr2;
} SpriteAttributes;

SpriteAttributes g_8x8SpriteAttributes;
SpriteAttributes g_16x8SpriteAttributes;
SpriteAttributes g_16x16SpriteAttributes;

dl_u16 playerSpriteTileIndex;
dl_u16 dropSpriteTileIndex;
dl_u16 cursorSpriteTileIndex;
dl_u16 ballSpriteTileIndex;

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

dl_u16 g_oamIndex = 0;

dl_u16 loadResourceToTiles(const dl_u8* sprite, 
						   dl_u8 width, 
						   dl_u8 height, 
						   dl_u8 spriteCount,
						   dl_u16 tileIndex)
{
	dl_u8 convertedSprite[256];
	memset(convertedSprite, 0, sizeof(convertedSprite));

	convert1bppImageTo8bppCrtEffectImage(sprite,
                                         convertedSprite,
                                         width,
                                         height,
                                         CrtColor_Blue);

	dl_u16 tileCount = convertToTiles(convertedSprite, 
									  width, 
									  height, 
									  CHAR_BASE_BLOCK(4),
									  tileIndex * 64); // 64 bytes after the last sprite

	return tileIndex + tileCount;
}

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	dl_u32 cursorSprite = 0xffffffff;

	// load tile resources
	dropSpriteTileIndex = 0;
	playerSpriteTileIndex = loadResourceToTiles(resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT, dropSpriteTileIndex);
	cursorSpriteTileIndex = loadResourceToTiles(resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT, playerSpriteTileIndex);
	ballSpriteTileIndex = loadResourceToTiles((dl_u8*)&cursorSprite, 8, 1, 1, cursorSpriteTileIndex);
	loadResourceToTiles(resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT, ballSpriteTileIndex);

	//playerSpriteTileIndex = 4;
	//cursorSpriteTileIndex = 12;

	g_8x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_8x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_8x8SpriteAttributes.attr2 = 0;

	g_16x16SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_16x16SpriteAttributes.attr1 = ATTR1_SIZE_16;
	g_16x16SpriteAttributes.attr2 = 0;

	g_16x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_WIDE;
	g_16x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_16x8SpriteAttributes.attr2 = 0;

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
    m_drawRoomFunctions[WIPE_TRANSITION_ROOM_INDEX] = drawTransition;//drawWipeTransition;
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
	g_oamIndex = 0;

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);

	for (int i = g_oamIndex; i < 128; i++) 
	{
		OAM[i].attr0 = ATTR0_DISABLED;
		OAM[i].attr1 = 0;
		OAM[i].attr2 = 0;
	}
}

void drawSprite(dl_u16 x, dl_u16 y, const SpriteAttributes* spriteAttributes, dl_u16 tileIndex)
{
	OAM[g_oamIndex].attr0 = spriteAttributes->attr0 | (y & 0xff);
	OAM[g_oamIndex].attr1 = spriteAttributes->attr1 | (x & 0x1ff);
	OAM[g_oamIndex].attr2 = spriteAttributes->attr2 | (tileIndex << 1);

	g_oamIndex++;
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
					   &g_8x8SpriteAttributes,
					   dropSpriteTileIndex);
        }

        dropsRunner++;
    }
}



void drawChamber(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);

    // draw ball
    if (gameData->ballData.enabled)
    {
        const BallData* ballData = &gameData->ballData;

		drawSprite((ballData->x >> 8) << 1,
				   ballData->y >> 8,
				   &g_16x8SpriteAttributes,
				   ballSpriteTileIndex); // ((dl_s8)ballData->fallStateCounter < 0));
    }
}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);

	drawSprite(gameData->numPlayers == 1 ? 32 : 128,
			   123,
			   &g_8x8SpriteAttributes,
			   cursorSpriteTileIndex);
}

void clearBackground()
{

}

void drawCleanBackground(dl_u8* cleanBackground)
{
	dl_u16* vramTileAddr = (dl_u16*)VRAM;
	dl_u16* vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(31);

	convertBackgroundToVRAM16(cleanBackground,
                              vramTileAddr,
							  vramTileMapAddr,
                              FRAMEBUFFER_WIDTH,
                              FRAMEBUFFER_HEIGHT,
                              CrtColor_Blue);
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
	drawDrops(gameData);
}
