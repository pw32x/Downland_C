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

dl_u8* g_cleanBackground;

void eraseSprite(dl_u16 x, dl_u16 y, dl_u8 width, dl_u8 height);

typedef struct
{
	dl_u16 x;
	dl_u16 y;
	const GameSprite* gameSprite;
} EraseEntry;

#define NUM_OBJECTS 48
EraseEntry g_eraseList0[NUM_OBJECTS];
EraseEntry g_eraseList1[NUM_OBJECTS];

dl_u8 g_eraseList0Count;
dl_u8 g_eraseList1Count;

EraseEntry* g_currentEraseList;
dl_u8* g_currentEraseListCount;

void initEraseList()
{
	g_eraseList0Count = 0;
	g_eraseList1Count = 0;

	g_currentEraseList = g_eraseList0;
	g_currentEraseListCount = &g_eraseList0Count;
}

void addToEraseList(dl_u16 x, dl_u16 y, const GameSprite* gameSprite)
{
	EraseEntry* currentEntry = &g_currentEraseList[(*g_currentEraseListCount)];
	currentEntry->x = x;
	currentEntry->y = y;
	currentEntry->gameSprite = gameSprite;
	(*g_currentEraseListCount)++;
}

void processEraseList()
{
	if (g_currentEraseList == g_eraseList0)
	{
		g_currentEraseList = g_eraseList1;
		g_currentEraseListCount = &g_eraseList1Count;
	}
	else
	{
		g_currentEraseList = g_eraseList0;
		g_currentEraseListCount = &g_eraseList0Count;
	}	

	EraseEntry* eraseListRunner = g_currentEraseList;
	for (dl_u8 loop = 0; loop < (*g_currentEraseListCount); loop++)
	{
		eraseSprite(eraseListRunner->x, 
					eraseListRunner->y,
					eraseListRunner->gameSprite->frameWidth,
					eraseListRunner->gameSprite->frameHeight);
		eraseListRunner++;
	}

	(*g_currentEraseListCount) = 0;
}

typedef void (*DrawRoomFunction)(GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(GameData* gameData, const Resources* resources);
void drawTitleScreen(GameData* gameData, const Resources* resources);
void drawTransition(GameData* gameData, const Resources* resources);
void drawWipeTransition(GameData* gameData, const Resources* resources);
void drawGetReadyScreen(GameData* gameData, const Resources* resources);

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
											  frameWidth,
											  0);

		spriteDataRunner += gameSprite->sizePerFrame;
		originalSpriteRunner += sizePerOriginalSpriteFrame;
	}
}

void buildTextResource(GameSprite* gameSprite,
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
											  frameWidth,
											  4);

		dl_u16 frameSize = frameWidth * frameHeight;
		for (int loop2 = 0; loop2 < frameSize; loop2++)
		{
			if (spriteDataRunner[loop2] == 0)
				spriteDataRunner[loop2] = 4;
			if (spriteDataRunner[loop2] == 3)
				spriteDataRunner[loop2] = 1;
		}

		spriteDataRunner += gameSprite->sizePerFrame;
		originalSpriteRunner += sizePerOriginalSpriteFrame;
	}
}


void buildPlayerIconResource(GameSprite* gameSprite,
							 const dl_u8* originalSprite, 
							 dl_u8 frameWidth, 
							 dl_u8 frameHeight, 
							 dl_u8 clipHeight,
							 dl_u8 spriteCount)
{
	dl_u16 spriteDataSize = frameWidth * frameHeight * spriteCount;
	dl_u8* spriteData = (dl_u8*)malloc(spriteDataSize);
	memset(spriteData, 0, sizeof(spriteDataSize));

	gameSprite->spriteData = spriteData;
	gameSprite->frameWidth = frameWidth;
	gameSprite->frameHeight = clipHeight;
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
											  frameWidth,
											  4);

		dl_u16 frameSize = frameWidth * frameHeight;
		for (int loop2 = 0; loop2 < frameSize; loop2++)
		{
			if (spriteDataRunner[loop2] == 0)
				spriteDataRunner[loop2] = 4;
		}

		spriteDataRunner += gameSprite->sizePerFrame;
		originalSpriteRunner += sizePerOriginalSpriteFrame;
	}
}

void buildEmptySpriteResource(GameSprite* gameSprite,
							  dl_u8 frameWidth, 
							  dl_u8 frameHeight, 
							  dl_u8 spriteCount)
{
	gameSprite->frameWidth = frameWidth;
	gameSprite->frameHeight = frameHeight;
	gameSprite->sizePerFrame = frameWidth * frameHeight;

	dl_u16 spriteDataSize = frameWidth * frameHeight * spriteCount;
	gameSprite->spriteData = (dl_u8*)malloc(spriteDataSize);
}

void updateRegenSprite(const Resources* resources, dl_u8 currentPlayerSpriteNumber)
{
	const dl_u16 bufferSize = (PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS;
	dl_u8 regenBuffer[bufferSize];
    memset(regenBuffer, 0, bufferSize);

    const dl_u8* originalSprite = resources->sprites_player;
    originalSprite += currentPlayerSpriteNumber * bufferSize;

    drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSprite, 
													PLAYER_SPRITE_ROWS,
													regenBuffer);

	convert1bppFramebufferTo8bppCrtEffect(regenBuffer,
										  regenSprite.spriteData,
										  PLAYER_SPRITE_WIDTH,
										  PLAYER_SPRITE_ROWS,
										  PLAYER_SPRITE_WIDTH,
										  0);

}


void GameRunner_ChangedRoomCallback(const GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);
void GameRunner_TransitionDone(const GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

#define SPLAT_FRAME_COUNT 2

void buildSplatSpriteResource(const Resources* resources)
{
	splatSprite.frameWidth = PLAYER_SPLAT_SPRITE_WIDTH;
	splatSprite.frameHeight = PLAYER_SPLAT_SPRITE_ROWS;
	splatSprite.sizePerFrame = PLAYER_SPLAT_SPRITE_WIDTH * PLAYER_SPLAT_SPRITE_ROWS;

	dl_u16 spriteDataSize = splatSprite.sizePerFrame * SPLAT_FRAME_COUNT;
	splatSprite.spriteData = (dl_u8*)malloc(spriteDataSize);

	dl_u8* spriteDataRunner = splatSprite.spriteData;

	for (dl_u8 loop = 0; loop < SPLAT_FRAME_COUNT; loop++)
	{
		convert1bppFramebufferTo8bppCrtEffect(resources->sprite_playerSplat,
											  spriteDataRunner,
											  splatSprite.frameWidth,
											  splatSprite.frameHeight,
											  splatSprite.frameWidth,
											  0);

		spriteDataRunner += splatSprite.sizePerFrame;
	}

	spriteDataRunner = splatSprite.spriteData + splatSprite.sizePerFrame;

	for (dl_u8 loop = 0; loop < 5 * splatSprite.frameWidth; loop++)
	{
		spriteDataRunner[loop] = 0;
	}
}

void GameRunner_Init(GameData* gameData, const Resources* resources)
{
	g_cleanBackground = (dl_u8*)malloc(FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

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

	playerIconSprite.spriteData = playerSprite.spriteData;
	playerIconSprite.frameWidth = playerSprite.frameWidth;
	playerIconSprite.frameHeight = PLAYERICON_NUM_SPRITE_ROWS;
	playerIconSprite.sizePerFrame = playerSprite.sizePerFrame;

	buildSplatSpriteResource(resources);

	// create the regen sprite for player icon
	playerIconSpriteRegen.spriteData = regenSprite.spriteData;
	playerIconSpriteRegen.frameWidth = regenSprite.frameWidth;
	playerIconSpriteRegen.frameHeight = PLAYERICON_NUM_SPRITE_ROWS;
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
    m_drawRoomFunctions[WIPE_TRANSITION_ROOM_INDEX] = drawWipeTransition;
    m_drawRoomFunctions[GET_READY_ROOM_INDEX] = drawGetReadyScreen;

	// Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;
	Game_TransitionDone = GameRunner_TransitionDone;


	initEraseList();

	Game_Init(gameData, resources);
}

void GameRunner_ChangedRoomCallback(const GameData* gameData, 
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



void GameRunner_Update(GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(GameData* gameData, const Resources* resources)
{
	processEraseList();

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);
}

// draw sprite, affected by scrolling
void drawSprite(dl_u16 x, 
				dl_u16 y, 
				dl_u8 frameIndex, 
				const GameSprite* gameSprite, 
				dl_u8 appendToEraseList)
{
	x += SCREEN_OFFSET_X;
	y += SCREEN_OFFSET_Y;

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

	if (appendToEraseList)
		addToEraseList(x, y, gameSprite);
}


void eraseSprite(dl_u16 x, 
				 dl_u16 y, 
				 dl_u8 width,
				 dl_u8 height)
{
	volatile unsigned char* frameBuffer = (unsigned char*)(&MARS_FRAMEBUFFER + 0x100);
	const dl_u8* cleanBackgroundRunner = g_cleanBackground;

	for (int loopy = 0; loopy < height; loopy++)
	{
		dl_u16 frameBufferOffsetY = (y + loopy) * SCREEN_WIDTH;
		dl_u16 cleanBackgroundOffsetY = (y + loopy - SCREEN_OFFSET_Y) * FRAMEBUFFER_WIDTH;

		for (int loopx = 0; loopx < width; loopx++)
		{
			frameBuffer[(x + loopx) + frameBufferOffsetY] = cleanBackgroundRunner[(x + loopx - SCREEN_OFFSET_X) + cleanBackgroundOffsetY];
		}
	}
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
					   &dropsSprite,
					   TRUE);
        }

        dropsRunner++;
    }
}

void drawUIText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 x = ((xyLocation % 32) * 8);
    dl_u16 y = (xyLocation / 32);

    // for each character
    while (*text != 0xff)
    {
        drawSprite(x, y, *text, &characterFont, FALSE);
        text++;
        x += 8;
    }
}

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = PLAYERLIVES_ICON_Y;

    for (dl_u8 loop = 0; loop < playerData->lives; loop++)
	{
        drawSprite(x << 1, 
				   y, 
				   playerData->currentSpriteNumber,
				   &playerIconSprite,
				   TRUE);

		x += PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
        drawSprite(x << 1, 
                   y, 
				   0,
				   &playerIconSpriteRegen,
				   TRUE);		
    }
}

void drawChamber(GameData* gameData, const Resources* resources)
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
				   &ballSprite,
				   TRUE);
    }

    // draw bird
    if (gameData->birdData.state && currentTimer == 0)
    {
        const BirdData* birdData = &gameData->birdData;

		drawSprite((birdData->x >> 8) << 1,
				   birdData->y >> 8,
				   birdData->animationFrame,
				   &birdSprite,
				   TRUE);
    }

	// draw player
    switch (playerData->state)
    {
    case PLAYER_STATE_SPLAT: 
        drawSprite(playerX,
                   playerY + 7,
                   playerData->splatFrameNumber,
				   &splatSprite,
				   TRUE);
        break;
    case PLAYER_STATE_REGENERATION: 

        if (!gameData->paused)
        {
			updateRegenSprite(resources, playerData->currentSpriteNumber);
        }

        drawSprite(playerX,
                   playerY,
                   0,
				   &regenSprite,
				   TRUE);
        break;
    default: 
        drawSprite(playerX,
                   playerY,
                   playerData->currentSpriteNumber,
				   &playerSprite,
				   TRUE);
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
					   g_pickUpSprites[pickups->type],
					   TRUE);
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
					   &doorSprite,
					   TRUE);
		}

		doorInfoRunner++;
	}

    // draw text
	drawUIText(gameData->string_timer, TIMER_DRAW_LOCATION);
	drawUIText(resources->text_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
	drawUIText(resources->text_chamber, CHAMBER_TEXT_DRAW_LOCATION);
	drawUIText(gameData->string_roomNumber, CHAMBER_NUMBER_TEXT_DRAW_LOCATION);
	drawUIText(playerData->scoreString, SCORE_DRAW_LOCATION);  

	drawUIPlayerLives(playerData);
}


void drawTitleScreen(GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);

	drawSprite(gameData->numPlayers == 1 ? 32 : 128,
			   123,
			   0,
			   &cursorSprite,
			   TRUE);
}

void drawCleanBackground(const GameData* gameData, 
						 const Resources* resources)
{
	convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
										  g_cleanBackground,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  FRAMEBUFFER_WIDTH,
										  4);


	dl_u32 offset = SCREEN_OFFSET_X + (SCREEN_OFFSET_Y * SCREEN_WIDTH);

	volatile unsigned char* _32XFramebuffer = (unsigned char*)(&MARS_FRAMEBUFFER + 0x100);
	convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
										  _32XFramebuffer + offset,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  SCREEN_WIDTH,
										  0);

	Mars_FlipFrameBuffers(1);

	convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
										  _32XFramebuffer + offset,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  SCREEN_WIDTH,
										  0);

	Mars_FlipFrameBuffers(1);

	initEraseList();
}

void drawTransition(GameData* gameData, const Resources* resources)
{
	if (gameData->transitionInitialDelay == 29)
    {
        drawCleanBackground(gameData, 
							resources);
    }
}

dl_u8 g_oldTransitionCurrentLine;

void drawTransitionLines(const GameData* gameData)
{
	for (int lineLoop = g_oldTransitionCurrentLine; lineLoop < gameData->transitionCurrentLine; lineLoop++)
	{
		dl_u32 framebufferOffset = (SCREEN_OFFSET_X + ((SCREEN_OFFSET_Y + lineLoop) * SCREEN_WIDTH)) >> 1;
		dl_u32 cleanBackgroundOffset = (lineLoop * FRAMEBUFFER_WIDTH) >> 1;

		const dl_u16* cleanBackground16 = (const dl_u16*)g_cleanBackground + cleanBackgroundOffset;

		volatile dl_u16* _32XFramebuffer = (dl_u16*)(&MARS_FRAMEBUFFER + 0x100);
		_32XFramebuffer += framebufferOffset;

		for (dl_u32 loop = 0; loop < 6; loop++)
		{
			for (dl_u16 innerLoop = 0; innerLoop < 128; innerLoop++)
			{
				_32XFramebuffer[innerLoop] = cleanBackground16[innerLoop];
			}

			_32XFramebuffer += (32 * 160);
			cleanBackground16 += (32 * 128);
		}
	}

	if (gameData->transitionCurrentLine == 32)
		return;

	dl_u32 framebufferOffset = (SCREEN_OFFSET_X + ((SCREEN_OFFSET_Y + gameData->transitionCurrentLine) * SCREEN_WIDTH)) >> 1;
	volatile dl_u16* _32XFramebuffer = (dl_u16*)(&MARS_FRAMEBUFFER + 0x100);
	_32XFramebuffer += framebufferOffset;

	for (dl_u32 loop = 0; loop < 6; loop++)
	{
		for (dl_u16 innerLoop = 0; innerLoop < 128; innerLoop++)
		{
			_32XFramebuffer[innerLoop] = 0x0101;
		}

		_32XFramebuffer += (32 * 160);
	}
}

void drawWipeTransition(GameData* gameData, const Resources* resources)
{
	if (gameData->transitionInitialDelay == 29)
    {
		g_oldTransitionCurrentLine = 0;

		convert1bppFramebufferTo8bppCrtEffect(gameData->cleanBackground, 
											  g_cleanBackground,
											  FRAMEBUFFER_WIDTH,
											  FRAMEBUFFER_HEIGHT,
											  FRAMEBUFFER_WIDTH,
											  4);
	}

	if (gameData->transitionInitialDelay)
		return;

	drawTransitionLines(gameData);
	Mars_FlipFrameBuffers(1);
	drawTransitionLines(gameData);
	
	// wait counter
	dl_u32 counter = 400;
	while (counter) 
	{
		counter--;
		Mars_FlipFrameBuffers(1);
	}


	g_oldTransitionCurrentLine = gameData->transitionCurrentLine;
}

void GameRunner_TransitionDone(const GameData* gameData, 
							   dl_u8 roomNumber, 
							   dl_s8 transitionType)
{
	if (transitionType == WIPE_TRANSITION_ROOM_INDEX)
	{
		drawTransitionLines(gameData);
		Mars_FlipFrameBuffers(1);
		drawTransitionLines(gameData);
		Mars_FlipFrameBuffers(1);
	}
}

void drawGetReadyScreen(GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
