#include "game_runner.h"

#include <gba_sprites.h>

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

	dl_u16 tileCount = convertToTiles(convertedSprite, width, height, tileIndex * 32); // 32 bytes after the last sprite

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

	Game_Init(gameData, resources);
}

void GameRunner_Update(struct GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(struct GameData* gameData, const Resources* resources)
{
	// draw
	dl_u16 oamIndex = 0;

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
