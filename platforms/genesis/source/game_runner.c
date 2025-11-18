#include "game_runner.h"

#include <genesis.h>
#include "kdebug.h"

#include "tilemaps.h"
#define TILE_MAP_WIDTH (FRAMEBUFFER_WIDTH / 8)
#define TILE_MAP_HEIGHT (FRAMEBUFFER_HEIGHT / 8)
#define TILE_MAP_SIZE (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)

// generated headers
#include "tileset.h"

#include "drops_manager.h"
#include "draw_utils.h"
#include "rooms/chambers.h"
#include "rooms/titlescreen.h"
#include "dl_rand.h"

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

#define X_OFFSET ((320 - FRAMEBUFFER_WIDTH) / 2)
#define Y_OFFSET ((224 - FRAMEBUFFER_HEIGHT) / 2)

typedef void (*RoomDrawFunction)(dl_u8 roomNumber, struct GameData* gameData, const Resources* resources);
RoomDrawFunction g_originalRoomDrawFunctions[NUM_ROOMS_AND_ALL];

void updateTransition(dl_u16 line);
void drawBlackTransitionScreen();

dl_u16 g_numSpritesToDraw = 0;

dl_u16 g_vdpTileIndex = 0;
dl_u16 g_transitionTilesetVDPIndex = 0;

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

void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

void drawTileText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 tilex = ((xyLocation % 32) * 8) / 8;
    dl_u16 tiley = (xyLocation / 32) / 8;

    // for each character
    while (*text != 0xff)
    {
		VDP_setTileMapXY(BG_B, characterFontSprite.vdpTileIndex + *text, tilex, tiley);

        text++;
        tilex++;
    }
}

void custom_chamber_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	drawBlackTransitionScreen();

	VDP_setTileMapDataRect(BG_B, roomTileMaps[roomNumber], 0, 0, TILE_MAP_WIDTH, TILE_MAP_HEIGHT, TILE_MAP_WIDTH, DMA);

	// init background and text
	g_originalRoomDrawFunctions[roomNumber](roomNumber, gameData, resources);

	if (!gameData->currentPlayerData->playerNumber)
		drawTileText(resources->text_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
	else
		drawTileText(resources->text_pl2, PLAYERLIVES_TEXT_DRAW_LOCATION);

	drawTileText(resources->text_chamber, CHAMBER_TEXT_DRAW_LOCATION);
	drawTileText(gameData->string_roomNumber, CHAMBER_NUMBER_TEXT_DRAW_LOCATION);

	PlayerData* playerData = gameData->playerData;
	convertScoreToString(playerData->score, playerData->scoreString);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];
	convertTimerToString(currentTimer,
						 gameData->string_timer);
	drawTileText(gameData->string_timer, TIMER_DRAW_LOCATION);
}

void custom_titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	drawBlackTransitionScreen();

	VDP_setTileMapDataRect(BG_B, roomTileMaps[roomNumber], 0, 0, TILE_MAP_WIDTH, TILE_MAP_HEIGHT, TILE_MAP_WIDTH, DMA);

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
	drawBlackTransitionScreen();

	VDP_setTileMapDataRect(BG_B, roomTileMaps[TITLESCREEN_ROOM_INDEX], 0, 0, TILE_MAP_WIDTH, TILE_MAP_HEIGHT, TILE_MAP_WIDTH, DMA);

	// init background and text
	g_originalRoomDrawFunctions[roomNumber](roomNumber, gameData, resources);

	// get ready text
	const dl_u8* getReadyString = gameData->currentPlayerData->playerNumber == PLAYER_ONE ? resources->text_getReadyPlayerOne : resources->text_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);
}

void GameRunner_Init(GameData* gameData, const Resources* resources)
{
	VDP_setVerticalScroll(BG_B, -Y_OFFSET);
	VDP_setHorizontalScroll(BG_B, X_OFFSET);
	VDP_setVerticalScroll(BG_A, -Y_OFFSET);
	VDP_setHorizontalScroll(BG_A, X_OFFSET);


	// load background
	VDP_loadTileSet(&backgroundTileset, 0, DMA);
	g_vdpTileIndex = backgroundTileset.numTile;
	VDP_loadTileSet(&transitionTileset, g_vdpTileIndex, DMA);
	g_transitionTilesetVDPIndex = g_vdpTileIndex;
	g_vdpTileIndex += transitionTileset.numTile;


	drawBlackTransitionScreen();

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

	buildSpriteResource(&playerIconSprite, &playerLivesTileset, PLAYER_SPRITE_WIDTH, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, SPRITE_SIZE(2,1));	
	buildSpriteResource(&playerIconSpriteRegen, &playerLivesRegenTileset, PLAYER_SPRITE_WIDTH, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, SPRITE_SIZE(2,1));	
	
	g_pickUpSprites[0] = &diamondSprite;
	g_pickUpSprites[1] = &moneyBagSprite;
	g_pickUpSprites[2] = &keySprite;


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


	Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;

	//setGameBackgroundTilemap(TITLESCREEN_ROOM_INDEX);

	Game_Init(gameData, resources);


	//createBackgrounds(gameData, resources);

}

void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType)
{
	return;
	if (roomNumber == WIPE_TRANSITION_ROOM_INDEX || 
		roomNumber == TRANSITION_ROOM_INDEX)
	{
		drawBlackTransitionScreen();
	}
}

void GameRunner_Update(GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

//void drawUIText(const dl_u8* text, dl_u16 x, dl_u16 y, GameSprite* font);

dl_u16 lineToTileIndex(dl_u16 line, dl_u16 range)
{
	if (line < range)
	{
		return TILE_ATTR_FULL(0, 1, 0, 0, g_transitionTilesetVDPIndex); // opaque tile
	}
	else if (line >= range && line < (range + 8))
	{
		return TILE_ATTR_FULL(0, 1, 0, 0, g_transitionTilesetVDPIndex + (line - range) + 1); // tile with line
	}

	return 0; // transparent tile
}

void computeTransitionTileIndexes(dl_u16* rowIndexes, dl_u16 line)
{
	rowIndexes[0] = lineToTileIndex(line, 0);
	rowIndexes[1] = lineToTileIndex(line, 8);
	rowIndexes[2] = lineToTileIndex(line, 16);
	rowIndexes[3] = lineToTileIndex(line, 24);
}

void updateTransition(dl_u16 line)
{
	dl_u16 rowIndexes[4];
	computeTransitionTileIndexes(rowIndexes, line);

	for (int loop = 0; loop < 24; loop++)
	{
		VDP_fillTileMapRect(BG_A, rowIndexes[loop & 3], 0, loop, 32, 1);
	}	
}

void drawBlackTransitionScreen()
{
	VDP_fillTileMapRect(BG_A, TILE_ATTR_FULL(0, 1, 0, 0, g_transitionTilesetVDPIndex), 0, 0, 32, 24);

}

void GameRunner_Draw(GameData* gameData, const Resources* resources)
{
	g_numSpritesToDraw = 0;
	VDP_clearSprites();

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);

	VDP_setSpriteLink(g_numSpritesToDraw - 1, 0);
	VDP_updateSprites(g_numSpritesToDraw, DMA);
}

// draw sprite, affected by scrolling
void drawSprite(dl_u16 x, dl_u16 y, dl_u8 frame, const GameSprite* gameSprite)
{
	dl_u16 tileIndex = gameSprite->vdpTileIndex + (frame * gameSprite->tilesPerFrame);

	VDP_setSpriteFull(g_numSpritesToDraw, 
					  x + X_OFFSET, 
					  y + Y_OFFSET, 
					  gameSprite->vdpSize, 
					  tileIndex, 
					  g_numSpritesToDraw + 1);
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
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = 0;//PLAYERLIVES_ICON_Y;

    for (dl_u8 loop = 0; loop < playerData->lives; loop++)
	{
        drawSprite(x << 1, 
				   y, 
				   playerData->currentSpriteNumber,
				   &playerIconSprite);

		x += PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
        drawSprite(x << 1, 
                   y, 
				   g_regenSpriteIndex,
				   &playerIconSpriteRegen);
    }
}

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
	drawTileText(gameData->string_timer, TIMER_DRAW_LOCATION);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);

	drawUIPlayerLives(playerData);
}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);


	drawSprite(gameData->numPlayers == 1 ? 32 : 128,
			   123,
			   0,
			   &cursorSprite);
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
	if (gameData->transitionInitialDelay == 29)
    {
		drawBlackTransitionScreen();
    }
    else if (!gameData->transitionInitialDelay)
    {
		updateTransition(32); // transparent
    }
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{
	if (gameData->transitionInitialDelay == 29)
    {
		drawBlackTransitionScreen();
    }

	if (!gameData->transitionCurrentLine)
		drawBlackTransitionScreen();
	else if (gameData->transitionCurrentLine < 31)
		updateTransition(gameData->transitionCurrentLine);
	else 
		updateTransition(32);
}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
