#include "game_runner.h"

#include <gba_sprites.h>
#include <gba_video.h>
#include <gba_systemcalls.h>

#include <string.h>

#include "image_utils.h"
#include "gba_defines.h"

#include "..\..\..\game\drops_manager.h"
#include "..\..\..\game\draw_utils.h"
#include "..\..\..\game\rooms\chambers.h"

typedef struct
{
	dl_u16 attr0;
	dl_u16 attr1;
	dl_u16 attr2;
} SpriteAttributes;

SpriteAttributes g_8x8SpriteAttributes;
SpriteAttributes g_16x8SpriteAttributes;
SpriteAttributes g_16x16SpriteAttributes;
SpriteAttributes g_textSpriteAttributes;
SpriteAttributes g_playerIconSpriteAttributes;

dl_s16 g_scrollx;
dl_s16 g_scrolly;


typedef struct
{
	const SpriteAttributes* spriteAttributes;
	dl_u16 tileIndex;
	dl_u8 tilesPerFrame;
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
GameSprite characterFont;
GameSprite playerIconSprite;
GameSprite playerIconSpriteRegen;

const GameSprite* g_pickUpSprites[3];

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
						 dl_u16 tileIndex)
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

		// add a dotted line at the bottom
		for (int loop2 = 56; loop2 < 64; loop2 += 2)
		{
			convertedSprite[loop2] = 4;
			convertedSprite[loop2 + 1] = 1;
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

	g_backgroundTileOffset = 2;

	// build tilemap for UI bar
	dl_u16* vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(UI_TILEMAP_INDEX);
	for (int loop = 0; loop < 30; loop++)
	{
		*vramTileMapAddr = 1;
		vramTileMapAddr++;
	}
}

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	// setup sprite attributes
	g_8x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_8x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_8x8SpriteAttributes.attr2 = OBJ_PRIORITY(2);

	g_16x16SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_16x16SpriteAttributes.attr1 = ATTR1_SIZE_16;
	g_16x16SpriteAttributes.attr2 = OBJ_PRIORITY(2);

	g_16x8SpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_WIDE;
	g_16x8SpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_16x8SpriteAttributes.attr2 = OBJ_PRIORITY(2);

	g_textSpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
	g_textSpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_textSpriteAttributes.attr2 = 0;

	g_playerIconSpriteAttributes.attr0 = ATTR0_COLOR_256 | ATTR0_WIDE;
	g_playerIconSpriteAttributes.attr1 = ATTR1_SIZE_8;
	g_playerIconSpriteAttributes.attr2 = 0;

	dl_u32 cursorSpriteRaw = 0xffffffff;

	// load tile resources
	dl_u16 tileIndex = 0;
	tileIndex = buildSpriteResource(&dropsSprite, &g_8x8SpriteAttributes, resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT, tileIndex);
	tileIndex = buildSpriteResource(&playerSprite, &g_16x16SpriteAttributes, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT, tileIndex);
	tileIndex = buildSpriteResource(&cursorSprite, &g_8x8SpriteAttributes, (dl_u8*)&cursorSpriteRaw, 8, 1, 1, tileIndex);
	tileIndex = buildSpriteResource(&ballSprite, &g_16x8SpriteAttributes, resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT, tileIndex);
	tileIndex = buildSpriteResource(&birdSprite, &g_16x8SpriteAttributes, resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT, tileIndex);
	tileIndex = buildSpriteResource(&keySprite, &g_16x16SpriteAttributes, resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, tileIndex);
	tileIndex = buildSpriteResource(&diamondSprite, &g_16x16SpriteAttributes, resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, tileIndex);
	tileIndex = buildSpriteResource(&moneyBagSprite, &g_16x16SpriteAttributes, resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1, tileIndex);
	tileIndex = buildSpriteResource(&doorSprite, &g_16x16SpriteAttributes, resources->sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1, tileIndex);
	tileIndex = buildEmptySpriteResource(&regenSprite, &g_16x16SpriteAttributes, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 1, tileIndex);
	tileIndex = buildTextResource(&characterFont, &g_textSpriteAttributes, resources->characterFont, 8, 7, 39, tileIndex);
	tileIndex = buildPlayerIconResource(&playerIconSprite, &g_playerIconSpriteAttributes, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, tileIndex);

	// load the splat tile twice to reserve the tile memory
	dl_u16 splatTileIndex = tileIndex;
	tileIndex = buildSpriteResource(&splatSprite, &g_16x16SpriteAttributes, resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_COUNT, splatTileIndex);
	buildSpriteResource(&splatSprite, &g_16x16SpriteAttributes, resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS, PLAYER_SPLAT_SPRITE_COUNT, tileIndex);
	splatSprite.tileIndex = splatTileIndex;

	// erase the first five rows of the second set of splat tiles
	dl_u16* vramAddress = (CHAR_BASE_BLOCK(4) + (tileIndex * 64));
	for (int loop = 0; loop < 5 * 4; loop++)
	{
		vramAddress[loop] = 0;
		vramAddress[loop + 32] = 0;
		vramAddress[loop + 64] = 0;
	}

	// create the regen sprite for player icon
	playerIconSpriteRegen.spriteAttributes = &g_playerIconSpriteAttributes;
	playerIconSpriteRegen.tileIndex = regenSprite.tileIndex;
	playerIconSpriteRegen.tilesPerFrame = regenSprite.tilesPerFrame;

	g_pickUpSprites[0] = &diamondSprite;
	g_pickUpSprites[1] = &moneyBagSprite;
	g_pickUpSprites[2] = &keySprite;

	g_scrollx = 7;
	g_scrolly = 13;

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

	buildUI();

	extern Room transitionRoom;
	g_rooms[WIPE_TRANSITION_ROOM_INDEX] = &transitionRoom;

	Game_Init(gameData, resources);
}

void GameRunner_Update(struct GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(struct GameData* gameData, const Resources* resources)
{
	BG_OFFSET[GAME_BACKGROUND_INDEX].x = g_scrollx;
	BG_OFFSET[GAME_BACKGROUND_INDEX].y = g_scrolly;

	g_oamIndex = 0;

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);

	for (int i = g_oamIndex; i < 128; i++) 
	{
		OAM[i].attr0 = ATTR0_DISABLED;
		OAM[i].attr1 = 0;
		OAM[i].attr2 = 0;
	}
}

// draw sprite, affected by scrolling
void drawSprite(dl_u16 x, dl_u16 y, dl_u8 frame, const GameSprite* gameSprite)
{
	x -= g_scrollx;
	y -= g_scrolly;
	dl_u16 tileIndex = gameSprite->tileIndex + (frame * gameSprite->tilesPerFrame);

	OAM[g_oamIndex].attr0 = gameSprite->spriteAttributes->attr0 | (y & 0xff);
	OAM[g_oamIndex].attr1 = gameSprite->spriteAttributes->attr1 | (x & 0x1ff);
	OAM[g_oamIndex].attr2 = gameSprite->spriteAttributes->attr2 | (tileIndex << 1);

	g_oamIndex++;
}

// draw sprite not affected by scrolling
void drawSpriteAbs(dl_u16 x, dl_u16 y, dl_u8 character, const GameSprite* gameSprite)
{
	dl_u16 tileIndex = gameSprite->tileIndex + (character * gameSprite->tilesPerFrame);

	OAM[g_oamIndex].attr0 = gameSprite->spriteAttributes->attr0 | (y & 0xff);
	OAM[g_oamIndex].attr1 = gameSprite->spriteAttributes->attr1 | (x & 0x1ff);
	OAM[g_oamIndex].attr2 = gameSprite->spriteAttributes->attr2 | (tileIndex << 1);

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
					   0,
					   &dropsSprite);
        }

        dropsRunner++;
    }
}

void drawUIText(const dl_u8* text, dl_u16 x, dl_u16 y)
{
    // for each character
    while (*text != 0xff)
    {
		drawSpriteAbs(x, y, *text, &characterFont);

        text++;
        x += 8;
    }
}

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = 80;//PLAYERLIVES_ICON_X;
	dl_u8 y = 0;//PLAYERLIVES_ICON_Y;

    for (dl_u8 loop = 0; loop < playerData->lives; loop++)
	{
        drawSpriteAbs(x, 
					  y, 
					  playerData->currentSpriteNumber,
					  &playerIconSprite);

		x += 24;//PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
        drawSpriteAbs(x, 
                      y, 
					  0,
					  &playerIconSpriteRegen);		
    }
}

void drawChamber(struct GameData* gameData, const Resources* resources)
{
	g_scrollx = 7;
	g_scrolly = 22;

	PlayerData* playerData = gameData->currentPlayerData;

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

	drawDrops(gameData);

    //dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

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
        drawSprite((playerData->x >> 8) << 1,
                   (playerData->y >> 8) + 7,
                   playerData->splatFrameNumber,
				   &splatSprite);
        break;
    case PLAYER_STATE_REGENERATION: 

        if (!gameData->paused)
        {
			updateRegenSprite(resources, playerData->currentSpriteNumber);
        }

        drawSprite((playerData->x >> 8) << 1,
                   playerData->y >> 8,
                   0,
				   &regenSprite);
        break;
    default: 
        drawSprite((playerData->x >> 8) << 1,
                   playerData->y >> 8,
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
        if (!(playerData->doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask))
            continue;

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

		doorInfoRunner++;
	}

    // draw text
	drawUIText(gameData->string_timer, 24 * 8, 0);
	drawUIText(playerData->scoreString, 8, 0); 

	drawUIPlayerLives(playerData);
}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	BG_OFFSET[GAME_BACKGROUND_INDEX].x = 7; 
	BG_OFFSET[GAME_BACKGROUND_INDEX].y = 13;

	drawDrops(gameData);

	drawSprite(gameData->numPlayers == 1 ? 32 : 128,
			   123,
			   0,
			   &cursorSprite);
}

void clearBackground()
{

}

void drawCleanBackground(dl_u8* cleanBackground, dl_u16 tileOffset)
{
	dl_u16* vramTileAddr = (dl_u16*)VRAM;
	dl_u16* vramTileMapAddr = (dl_u16*)MAP_BASE_ADR(GAME_TILEMAP_INDEX);

	convertBackgroundToVRAM16(cleanBackground,
                              vramTileAddr,
							  vramTileMapAddr,
							  tileOffset,
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
        drawCleanBackground(gameData->cleanBackground, g_backgroundTileOffset);
    }
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{

}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
