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
    0x7FFF  // white (R=31, G=31, B=31)
};

typedef void (*DrawRoomFunction)(struct GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(struct GameData* gameData, const Resources* resources);
void drawTitleScreen(struct GameData* gameData, const Resources* resources);
void drawTransition(struct GameData* gameData, const Resources* resources);
void drawWipeTransition(struct GameData* gameData, const Resources* resources);
void drawGetReadyScreen(struct GameData* gameData, const Resources* resources);


/*
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
*/


//void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

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

/*
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
*/

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

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	dl_u32 cursorSpriteRaw = 0xffffffff;

	/*
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
	tileIndex = buildTextResource(&characterFont, &g_textSpriteAttributes, resources->characterFont, 8, 7, 39, tileIndex, FALSE);
	tileIndex = buildPlayerIconResource(&playerIconSprite, &g_playerIconSpriteAttributes, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT, tileIndex);


	buildSplatSpriteResource(tileIndex, resources);

	// create the regen sprite for player icon
	playerIconSpriteRegen.spriteAttributes = &g_playerIconSpriteAttributes;
	playerIconSpriteRegen.tileIndex = regenSprite.tileIndex;
	playerIconSpriteRegen.tilesPerFrame = regenSprite.tilesPerFrame;
	*/


	InitGameSprite(&dropsSprite, resources->sprites_drops, DROP_SPRITE_WIDTH, DROP_SPRITE_ROWS, DROP_SPRITE_COUNT);
	InitGameSprite(&playerSprite,resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYER_SPRITE_COUNT);
	InitGameSprite(&cursorSprite,(dl_u8*)&cursorSpriteRaw, 8, 1, 1);
	InitGameSprite(&ballSprite, resources->sprites_bouncyBall, BALL_SPRITE_WIDTH, BALL_SPRITE_ROWS, BALL_SPRITE_COUNT);
	InitGameSprite(&birdSprite, resources->sprites_bird, BIRD_SPRITE_WIDTH, BIRD_SPRITE_ROWS, BIRD_SPRITE_COUNT);
	InitGameSprite(&keySprite, resources->sprite_key, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	InitGameSprite(&diamondSprite, resources->sprite_diamond, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	InitGameSprite(&moneyBagSprite, resources->sprite_moneyBag, PICKUPS_NUM_SPRITE_WIDTH, PICKUPS_NUM_SPRITE_ROWS, 1);
	InitGameSprite(&doorSprite, resources->sprite_door, DOOR_SPRITE_WIDTH, DOOR_SPRITE_ROWS, 1);
	//buildEmptySpriteResource(&regenSprite, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, 1);
	//buildTextResource(&hudCharacterFont, resources->characterFont, 8, 7, 39, tile);
	InitFontSprite(&characterFont, resources->characterFont, 8, 7, 39);
	//buildPlayerIconResource(&playerIconSprite, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, PLAYERICON_NUM_SPRITE_ROWS, PLAYER_SPRITE_COUNT);

	InitSplatSprite(&splatSprite, resources->sprite_playerSplat, PLAYER_SPLAT_SPRITE_WIDTH, PLAYER_SPLAT_SPRITE_ROWS);

	InitGameSprite(&g_crtFramebufferSprite, NULL, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 1);
    g_crtFramebufferSprite.ccb.ccb_Flags |= CCB_BGND; // make black pixels not transparent

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

	g_rooms[WIPE_TRANSITION_ROOM_INDEX]->update = g_rooms[TRANSITION_ROOM_INDEX]->update;

	//Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;


	Game_Init(gameData, resources);
}

/*
void GameRunner_ChangedRoomCallback(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType)
{
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
}
*/

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

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = PLAYERLIVES_ICON_Y;
	dl_u8 loop;

    for (loop = 0; loop < playerData->lives; loop++)
	{
        drawSprite(x, 
				   y, 
				   playerData->currentSpriteNumber,
				   &playerIconSprite);

		x += PLAYERLIVES_ICON_SPACING;
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
		/*
        if (!gameData->paused)
        {
			updateRegenSprite(resources, playerData->currentSpriteNumber);
        }

        drawSprite(playerX,
                   playerY,
                   0,
				   &regenSprite);
		*/
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
	drawUIText(resources->text_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
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
	if (!gameData->transitionInitialDelay)
    {
        drawCleanBackground(gameData, 
							resources);
    }

	drawSprite(0, 0, 0, &g_crtFramebufferSprite);
}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{
	if (!gameData->transitionInitialDelay)
    {
        drawCleanBackground(gameData, 
							resources);
    }

	drawSprite(0, 0, 0, &g_crtFramebufferSprite);

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
	drawSprite(0, 0, 0, &g_crtFramebufferSprite);
	drawDrops(gameData);
}
