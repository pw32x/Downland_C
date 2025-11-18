#include "game_runner.h"

// game includes
#include "draw_utils.h"
#include "rooms\chambers.h"
#include "string_utils.h"
#include "base_defines.h"

// project includes
#include "image_utils.h"
#include "drops_manager.h"
#include "display.h"
#include "3do_defines.h"

// 3do includes
#include "celutils.h"
#include "display.h"

typedef struct
{
	CCB ccb;
	// x, y, picx, pre0, pre1, flags
	dl_u8* spriteData;
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

GameSprite* g_pickUpSprites[3];

GameSprite g_crtFramebufferSprite;

dl_u16 g_gamePalette[4] = 
{
    0x0000, // black (R=0, G=0, B=0)
    0x003E, // blue (R=0, G=0, B=31)
    0x7E40, // orange (R=31, G=20, B=0)
    0x7FFF, // white (R=31, G=31, B=31)
};

dl_u16 g_blackPalette[4] = 
{
    0x0000,
    0x0000,
    0x0000,
    0x0000,
};

typedef void (*DrawRoomFunction)(struct GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(struct GameData* gameData, const Resources* resources);
void drawTitleScreen(struct GameData* gameData, const Resources* resources);
void drawTransition(struct GameData* gameData, const Resources* resources);
void drawWipeTransition(struct GameData* gameData, const Resources* resources);
void drawGetReadyScreen(struct GameData* gameData, const Resources* resources);

void updateRegenSprite(const Resources* resources, dl_u8 currentPlayerSpriteNumber)
{
#define ORIGINAL_SPRITE_FRAME_SIZE ((PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS)

	const dl_u8* originalSpriteFrame = resources->sprites_player + (currentPlayerSpriteNumber * ORIGINAL_SPRITE_FRAME_SIZE);
	dl_u8 regenBuffer[ORIGINAL_SPRITE_FRAME_SIZE];

	memset(regenBuffer, 0, ORIGINAL_SPRITE_FRAME_SIZE);

    drawSprite_16PixelsWide_static_IntoSpriteBuffer(originalSpriteFrame, 
													PLAYER_SPRITE_ROWS,
													regenBuffer);

	convert1bppImageTo2bppCrtEffectImage(regenBuffer,
										 (dl_u8*)regenSprite.ccb.ccb_SourcePtr,
										 PLAYER_SPRITE_WIDTH,
										 PLAYER_SPRITE_ROWS,
										 32, // totally hardcoded width
										 CrtColor_Blue);
}


void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

void InitSplatSprite(GameSprite* gameSprite, 
					 const dl_u8* originalSpriteData,
  					 dl_u16 frameWidth,
					 dl_u16 frameHeight)
{
	dl_u8 frameCount = 2;
	dl_u8 pixelsPerByte = 4;
	dl_u8 bytesPerRow = (frameWidth / pixelsPerByte);
	dl_u32 sizePerFrame;
	dl_u32 memSize;
	dl_u8* buffer;
	dl_u8* bufferRunner;
	dl_u16 actualFrameWidth;
	CCB* ccb;
	dl_u16 loop;
	dl_u16 originalSpriteFrameSize = (frameWidth / 8) * frameHeight;

	if (bytesPerRow < 8)
		bytesPerRow = 8;

	actualFrameWidth = bytesPerRow * pixelsPerByte;

	sizePerFrame = bytesPerRow * frameHeight;
	memSize = sizePerFrame * frameCount;
	buffer = (dl_u8*)malloc(memSize);

	ccb = &gameSprite->ccb;

    InitCel(ccb, frameWidth, frameHeight, 2, INITCEL_CODED);
    ccb->ccb_PLUTPtr = g_gamePalette;
	ccb->ccb_SourcePtr = (CelData *)buffer;

	gameSprite->spriteData = buffer;
	gameSprite->sizePerFrame = sizePerFrame;

	if (originalSpriteData != NULL)
	{
		bufferRunner = buffer;
		for (loop = 0; loop < frameCount; loop++)
		{
			convert1bppImageTo2bppCrtEffectImage(originalSpriteData,
												 (dl_u8*)bufferRunner,
												 frameWidth,
												 frameHeight,
												 actualFrameWidth,
												 CrtColor_Blue);

			bufferRunner += sizePerFrame;
			// originalSpriteData += originalSpriteFrameSize; // use the same frame twice
		}
	}

	// erase the first five rows of the second frame
	bufferRunner = buffer + sizePerFrame;
	for (loop = 0; loop < 5 * 8; loop++)
	{
		bufferRunner[loop] = 0;
	}
}

void InitGameSprite(GameSprite* gameSprite, 
					const dl_u8* originalSpriteData,
					dl_u16 frameWidth, 
					dl_u16 frameHeight, 
					dl_u8 frameCount)
{
	dl_u8 pixelsPerByte = 4;
	dl_u8 bytesPerRow = (frameWidth / pixelsPerByte);
	dl_u32 sizePerFrame;
	dl_u32 memSize;
	dl_u8* buffer;
	dl_u8* bufferRunner;
	dl_u16 actualFrameWidth;
	CCB* ccb;
	dl_u8 loop;
	dl_u16 originalSpriteFrameSize = (frameWidth / 8) * frameHeight;

	if (bytesPerRow < 8)
		bytesPerRow = 8;

	actualFrameWidth = bytesPerRow * pixelsPerByte;

	sizePerFrame = bytesPerRow * frameHeight;
	memSize = sizePerFrame * frameCount;
	buffer = (dl_u8*)malloc(memSize);

	ccb = &gameSprite->ccb;

    InitCel(ccb, frameWidth, frameHeight, 2, INITCEL_CODED);
    ccb->ccb_PLUTPtr = g_gamePalette;
	ccb->ccb_SourcePtr = (CelData *)buffer;

	gameSprite->spriteData = buffer;
	gameSprite->sizePerFrame = sizePerFrame;

	if (originalSpriteData != NULL)
	{
		bufferRunner = buffer;
		for (loop = 0; loop < frameCount; loop++)
		{
			convert1bppImageTo2bppCrtEffectImage(originalSpriteData,
												 (dl_u8*)bufferRunner,
												 frameWidth,
												 frameHeight,
												 actualFrameWidth,
												 CrtColor_Blue);

			bufferRunner += sizePerFrame;
			originalSpriteData += originalSpriteFrameSize;
		}
	}
}

void InitFontSprite(GameSprite* gameSprite, 
					const dl_u8* originalSpriteData,
					dl_u16 frameWidth, 
					dl_u16 frameHeight, 
					dl_u8 frameCount)
{
	dl_u8 pixelsPerByte = 4;
	dl_u8 bytesPerRow = (frameWidth / pixelsPerByte);
	dl_u32 sizePerFrame;
	dl_u32 memSize;
	dl_u8* buffer;
	dl_u8* bufferRunner;
	dl_u16 actualFrameWidth;
	CCB* ccb;
	dl_u8 loop;
	dl_u16 originalSpriteFrameSize = (frameWidth / 8) * frameHeight;

	if (bytesPerRow < 8)
		bytesPerRow = 8;

	actualFrameWidth = bytesPerRow * pixelsPerByte;

	sizePerFrame = bytesPerRow * frameHeight;
	memSize = sizePerFrame * frameCount;
	buffer = (dl_u8*)malloc(memSize);

	ccb = &gameSprite->ccb;

    InitCel(ccb, frameWidth, frameHeight, 2, INITCEL_CODED);
    ccb->ccb_PLUTPtr = g_gamePalette;
	ccb->ccb_SourcePtr = (CelData *)buffer;

	gameSprite->spriteData = buffer;
	gameSprite->sizePerFrame = sizePerFrame;

	if (originalSpriteData != NULL)
	{
		bufferRunner = buffer;
		for (loop = 0; loop < frameCount; loop++)
		{
			convert1bppImageTo2bppBlueImage(originalSpriteData,
										    (dl_u8*)bufferRunner,
										    frameWidth,
										    frameHeight,
										    actualFrameWidth);

			bufferRunner += sizePerFrame;
			originalSpriteData += originalSpriteFrameSize;
		}
	}
}

dl_u8 transitionSpriteData[] = 
{
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

CCB transitionCCB;
void InitTransitionSprite()
{
    InitCel(&transitionCCB, 16, 32, 4, INITCEL_CODED);
	transitionCCB.ccb_Flags |= CCB_BGND;
    transitionCCB.ccb_PLUTPtr = g_gamePalette;

	transitionCCB.ccb_HDX = 256 << 16;  // 8.0 fixed point
    transitionCCB.ccb_HDY = 0;
    transitionCCB.ccb_VDX = 0;
    transitionCCB.ccb_VDY = 1 << 16;  // no vertical scaling

	transitionCCB.ccb_SourcePtr = (CelData *)transitionSpriteData;
}

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	dl_u32 cursorSpriteRaw = 0xffffffff;

	InitGameSprite(&dropsSprite, resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT);
	InitGameSprite(&playerSprite,resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT);
	InitGameSprite(&cursorSprite,(dl_u8*)&cursorSpriteRaw, 8, 1, 1);
	InitGameSprite(&ballSprite, resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT);
	InitGameSprite(&birdSprite, resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT);
	InitGameSprite(&keySprite, resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	InitGameSprite(&diamondSprite, resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	InitGameSprite(&moneyBagSprite, resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	InitGameSprite(&doorSprite, resources->sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1);
	InitGameSprite(&regenSprite, NULL, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 1);
	InitFontSprite(&characterFont, resources->characterFont, 8, 7, 39);
	//buildPlayerIconResource(&playerIconSprite, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT);

	InitSplatSprite(&splatSprite, resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS);

	InitGameSprite(&g_crtFramebufferSprite, NULL, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 1);
    g_crtFramebufferSprite.ccb.ccb_Flags |= CCB_BGND; // make black pixels not transparent

	InitTransitionSprite();

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

	//g_rooms[WIPE_TRANSITION_ROOM_INDEX]->update = g_rooms[TRANSITION_ROOM_INDEX]->update;

	Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;

	Game_Init(gameData, resources);
}


void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType)
{
	transitionCCB.ccb_PLUTPtr = g_blackPalette;
}


void GameRunner_Update(struct GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(struct GameData* gameData, const Resources* resources)
{
	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);
}

// draw sprite, affected by scrolling
void drawSprite(dl_u16 x, dl_u16 y, dl_u8 frameNumber, GameSprite* gameSprite)
{
	gameSprite->ccb.ccb_XPos = ((x + SCREEN_OFFSET_X) << 16);
	gameSprite->ccb.ccb_YPos = ((y + SCREEN_OFFSET_Y) << 16);

	gameSprite->ccb.ccb_SourcePtr = (CelData *)(gameSprite->spriteData + (gameSprite->sizePerFrame * frameNumber));

	draw_cels(&gameSprite->ccb);
}

void drawDrops(const GameData* gameData)
{
    // draw drops
	int loop;
    const Drop* dropsRunner = gameData->dropData.drops;

    for (loop = 0; loop < NUM_DROPS; loop++)
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

void drawUIText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 x = ((xyLocation % 32) * 8);
    dl_u16 y = (xyLocation / 32);

    // for each character
    while (*text != 0xff)
    {
        drawSprite(x, y, *text, &characterFont);
        text++;
        x += 8;
    }
}

#define SET_PRE0_HEIGHT(ccb_PRE0, height)  (ccb_PRE0 & ~0xFC0) | ((height - 1) << 6)

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = PLAYERLIVES_ICON_Y;
	dl_u8 loop;

	int32 spritePRE0 = playerSprite.ccb.ccb_PRE0;
	playerSprite.ccb.ccb_PRE0 = SET_PRE0_HEIGHT(playerSprite.ccb.ccb_PRE0, PLAYERICON_NUM_SPRITE_ROWS);
	regenSprite.ccb.ccb_PRE0 = SET_PRE0_HEIGHT(regenSprite.ccb.ccb_PRE0, PLAYERICON_NUM_SPRITE_ROWS);

    for (loop = 0; loop < playerData->lives; loop++)
	{
        drawSprite(x << 1, 
				   y, 
				   playerData->currentSpriteNumber,
				   &playerSprite);

		x += PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
        drawSprite(x << 1, 
                   y, 
				   0,
				   &regenSprite);		
    }   

	playerSprite.ccb.ccb_PRE0 = spritePRE0;
	regenSprite.ccb.ccb_PRE0 = spritePRE0;
}

void drawChamber(struct GameData* gameData, const Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;
	dl_u16 playerX = (playerData->x >> 8) << 1;
	dl_u16 playerY = (playerData->y >> 8);
	const BallData* ballData = &gameData->ballData;
	const BirdData* birdData = &gameData->birdData;
	const Pickup* pickups;
	int roomNumber;
	const DoorInfoData* doorInfoData;
	const DoorInfo* doorInfoRunner;
	dl_u8 loop;
	int xPosition;
	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

	drawSprite(0, 0, 0, &g_crtFramebufferSprite);

	drawDrops(gameData);


    // draw ball
    if (gameData->ballData.enabled)
    {
		drawSprite((ballData->x >> 8) << 1,
				   ballData->y >> 8,
				   (dl_s8)ballData->fallStateCounter < 0,
				   &ballSprite);
    }

    // draw bird
    if (gameData->birdData.state && currentTimer == 0)
    {
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
    pickups = playerData->gamePickups[gameData->currentRoom->roomNumber];
    for (loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
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
    roomNumber = gameData->currentRoom->roomNumber;
	doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
	doorInfoRunner = doorInfoData->doorInfos;

	for (loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
        if (playerData->doorStateData[doorInfoRunner->globalDoorIndex] & playerData->playerMask &&
			doorInfoRunner->x != 0xff)
		{
			xPosition = doorInfoRunner->x;
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
	drawUIText(gameData->string_timer, TIMER_DRAW_LOCATION);

	if (!gameData->currentPlayerData->playerNumber)
		drawUIText(resources->text_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
	else
		drawUIText(resources->text_pl2, PLAYERLIVES_TEXT_DRAW_LOCATION);

	drawUIText(resources->text_chamber, CHAMBER_TEXT_DRAW_LOCATION);
	drawUIText(gameData->string_roomNumber, CHAMBER_NUMBER_TEXT_DRAW_LOCATION);
	drawUIText(playerData->scoreString, SCORE_DRAW_LOCATION);  

	drawUIPlayerLives(playerData);
}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	drawSprite(0, 0, 0, &g_crtFramebufferSprite);

	drawDrops(gameData);

	drawSprite(gameData->numPlayers == 1 ? 32 : 128,
			   123,
			   0,
			   &cursorSprite);
}


void drawCleanBackground(const GameData* gameData, 
						 const Resources* resources)
{
	convert1bppImageTo2bppCrtEffectImage(gameData->cleanBackground,
                                         (dl_u8*)g_crtFramebufferSprite.spriteData,
                                         FRAMEBUFFER_WIDTH,
                                         FRAMEBUFFER_HEIGHT,
										 FRAMEBUFFER_WIDTH,
                                         CrtColor_Blue);
}

void drawTransition(struct GameData* gameData, const Resources* resources)
{
	dl_u8 loop = 0;
	dl_u16 x = 0;
	dl_u16 y = 0;

	if (gameData->transitionInitialDelay == 29)
    {
        drawCleanBackground(gameData, 
							resources);
    }
	else if (!gameData->transitionInitialDelay)
	{
		transitionCCB.ccb_PLUTPtr = g_gamePalette;
	}

	drawSprite(0, 0, 0, &g_crtFramebufferSprite);

	for (loop = 0; loop < 6; loop++)
	{
		transitionCCB.ccb_XPos = ((x + SCREEN_OFFSET_X) << 16);
		transitionCCB.ccb_YPos = ((y + SCREEN_OFFSET_Y) << 16);
		transitionCCB.ccb_PRE0 = SET_PRE0_HEIGHT(transitionCCB.ccb_PRE0, 32);
		draw_cels(&transitionCCB);

		y += 32;
	}
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{
	dl_u8 loop = 0;
	dl_u16 x = 0;
	dl_u16 y = gameData->transitionCurrentLine;

	if (gameData->transitionInitialDelay == 29)
    {
        drawCleanBackground(gameData, 
							resources);
    }
	else if (!gameData->transitionInitialDelay)
	{
		transitionCCB.ccb_PLUTPtr = g_gamePalette;
	}

	drawSprite(0, 0, 0, &g_crtFramebufferSprite);

	for (loop = 0; loop < 6; loop++)
	{
		transitionCCB.ccb_XPos = ((x + SCREEN_OFFSET_X) << 16);
		transitionCCB.ccb_YPos = ((y + SCREEN_OFFSET_Y) << 16);
		transitionCCB.ccb_PRE0 = SET_PRE0_HEIGHT(transitionCCB.ccb_PRE0, 32 - gameData->transitionCurrentLine);
		draw_cels(&transitionCCB);

		y += 32;
	}
}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{
	drawSprite(0, 0, 0, &g_crtFramebufferSprite);
	drawDrops(gameData);
}
