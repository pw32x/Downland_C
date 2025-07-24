#include "game_runner.h"

// platform headers
#include "mars.h"

// std headers
#include <string.h>
#include <stdlib.h>

// project headers		
#include "32x_defines.h"
#include "image_utils.h"

// game headers
#include "drops_manager.h"
#include "draw_utils.h"
#include "rooms/chambers.h"

typedef struct
{
	dl_u8* spriteData;
	dl_u8 frameWidth;
	dl_u8 frameHeight;
	dl_u16 sizePerFrame;
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

dl_u8* cleanBackground;

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

void buildSpriteResource(GameSprite* gameSprite,
						 const dl_u8* originalSprite, 
						 dl_u8 frameWidth, 
						 dl_u8 frameHeight, 
						 dl_u8 spriteCount)
{
	dl_u16 spriteDataSize = frameWidth * frameHeight * spriteCount;
	dl_u8* spriteData = (dl_u8*)malloc(spriteDataSize);
	memset(spriteData, 0, sizeof(spriteDataSize));

	gameSprite->spriteData = spriteData;
	gameSprite->frameWidth = frameWidth;
	gameSprite->frameHeight = frameHeight;
	gameSprite->sizePerFrame = frameWidth * frameHeight;

	dl_u8* spriteDataRunner = spriteData;
	const dl_u8* originalSpriteRunner = originalSprite;

	dl_u16 sizePerOriginalSpriteFrame = (frameWidth / 8) * frameHeight;

	for (int loop = 0; loop < spriteCount; loop++)
    {
		convert1bppFramebufferTo8bppCrtEffect(originalSpriteRunner,
											  spriteDataRunner,
											  frameWidth,
											  frameHeight,
											  frameWidth);

		spriteDataRunner += gameSprite->sizePerFrame;
		originalSpriteRunner += sizePerOriginalSpriteFrame;
	}
}

dl_u16 buildTextResource(GameSprite* gameSprite,
						 const dl_u8* originalSprite, 
						 dl_u8 width, 
						 dl_u8 height, 
						 dl_u8 spriteCount)
{
	/*
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
	*/

	return 0;
}

dl_u16 buildPlayerIconResource(GameSprite* gameSprite,
							   const dl_u8* originalSprite, 
							   dl_u8 width, 
							   dl_u8 height, 
							   dl_u8 clipHeight,
							   dl_u8 spriteCount)
{
	/*
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
	*/

	return 0;
}

dl_u16 buildEmptySpriteResource(GameSprite* gameSprite,
								dl_u8 width, 
								dl_u8 height, 
								dl_u8 spriteCount)
{
	/*
	gameSprite->spriteAttributes = spriteAttributes;
	gameSprite->tileIndex = tileIndex;
	gameSprite->tilesPerFrame = ((width + 7) / 8) * ((height + 7) / 8);

	tileIndex += gameSprite->tilesPerFrame;

	return tileIndex;
	*/

	return 0;
}

void updateRegenSprite(const Resources* resources, dl_u8 currentPlayerSpriteNumber)
{
	/*
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
*/
}


void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

void buildSplatSpriteResource(const Resources* resources)
{
	/*
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
	*/
}

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	cleanBackground = (dl_u8*)malloc(FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

	dl_u32 cursorSpriteRaw = 0xffffffff;

	// load tile resources
	buildSpriteResource(&dropsSprite, resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT);
	buildSpriteResource(&playerSprite, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT);
	buildSpriteResource(&cursorSprite, (dl_u8*)&cursorSpriteRaw, 8, 1, 1);
	buildSpriteResource(&ballSprite, resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT);
	buildSpriteResource(&birdSprite, resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT);
	buildSpriteResource(&keySprite, resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	buildSpriteResource(&diamondSprite, resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	buildSpriteResource(&moneyBagSprite, resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	buildSpriteResource(&doorSprite, resources->sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1);
	buildEmptySpriteResource(&regenSprite, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 1);
	buildTextResource(&characterFont, resources->characterFont, 8, 7, 39);
	buildPlayerIconResource(&playerIconSprite, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT);

	buildSplatSpriteResource(resources);

	// create the regen sprite for player icon
	playerIconSpriteRegen.spriteData = NULL;
	playerIconSpriteRegen.frameWidth = PLAYER_SPRITE_WIDTH;
	playerIconSpriteRegen.frameHeight = PLAYER_SPRITE_ROWS;
	playerIconSpriteRegen.sizePerFrame = PLAYER_SPRITE_WIDTH * PLAYER_SPRITE_ROWS;

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
    m_drawRoomFunctions[WIPE_TRANSITION_ROOM_INDEX] = drawTransition;//drawWipeTransition;
    m_drawRoomFunctions[GET_READY_ROOM_INDEX] = drawGetReadyScreen;

	// Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;

	Game_Init(gameData, resources);
}

void GameRunner_ChangedRoomCallback(const struct GameData* gameData, 
									dl_u8 roomNumber, 
									dl_s8 transitionType)
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

void GameRunner_Update(struct GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void drawUIText(const dl_u8* text, dl_u16 x, dl_u16 y, GameSprite* font);

void GameRunner_Draw(struct GameData* gameData, const Resources* resources)
{
	// switch to other set of sprites to erase
	// erase sprites for this frame

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);
}

// draw sprite, affected by scrolling
void drawSprite(dl_u16 x, dl_u16 y, dl_u8 frameIndex, const GameSprite* gameSprite)
{
	const dl_u8* spriteData = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);

	volatile unsigned char* frameBuffer = (unsigned char*)(&MARS_FRAMEBUFFER + 0x100);

	for (int loopy = 0; loopy < gameSprite->frameHeight; loopy++)
	{
		dl_u16 offsetY = (y + loopy) * SCREEN_WIDTH;

		for (int loopx = 0; loopx < gameSprite->frameWidth; loopx++)
		{
			dl_u8 pixel = *spriteData;

			if (pixel)
				frameBuffer[(x + loopx) + offsetY] = pixel;

			spriteData++;
		}
	}

	/*
	x -= g_scrollX;
	y -= g_scrollY;
	dl_u16 tileIndex = gameSprite->tileIndex + (frame * gameSprite->tilesPerFrame);

	OAM[g_oamIndex].attr0 = gameSprite->spriteAttributes->attr0 | (y & 0xff);
	OAM[g_oamIndex].attr1 = gameSprite->spriteAttributes->attr1 | (x & 0x1ff);
	OAM[g_oamIndex].attr2 = gameSprite->spriteAttributes->attr2 | (tileIndex << 1);

	g_oamIndex++;
	*/
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
}

void drawChamber(struct GameData* gameData, const Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;
	dl_u16 playerX = (playerData->x >> 8) << 1;
	dl_u16 playerY = (playerData->y >> 8);

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
			updateRegenSprite(resources, playerData->currentSpriteNumber);
        }

        drawSprite(playerX,
                   playerY,
                   0,
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
	drawUIText(gameData->string_timer, 24 * 8, 0, &characterFont);
	drawUIText(playerData->scoreString, 8, 0, &characterFont); 

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

void drawCleanBackground(const GameData* gameData, 
						 const Resources* resources)
{
	convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
										  cleanBackground,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  FRAMEBUFFER_WIDTH);


	volatile unsigned char* _32XFramebuffer = (unsigned char*)(&MARS_FRAMEBUFFER + 0x100);
	convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
										  _32XFramebuffer,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  SCREEN_WIDTH);

	Mars_FlipFrameBuffers(1);

	convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
										  _32XFramebuffer,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  SCREEN_WIDTH);

	Mars_FlipFrameBuffers(1);
}

void drawTransition(struct GameData* gameData, const Resources* resources)
{
	if (gameData->transitionInitialDelay == 29)
    {
        drawCleanBackground(gameData, 
							resources);
    }
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{
	if (gameData->transitionInitialDelay == 29)
    {
        drawCleanBackground(gameData, 
							resources);
    }
}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
