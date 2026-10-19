#include "game_runner.h"

// platform headers
#include "cdi_hw/video.h"
#include "cdi_hw/graphics.h"
#include "draw_erase/draw_sprite_384.h"
#include "draw_erase/draw_sprite_256.h"
#include "draw_erase/erase_sprite_utils.h"
#include "draw_erase/erase_system.h"
#include "draw_erase/draw_value.h"
#include "build_resource_utils.h"

// project headers
#include "image_utils.h"

// game headers
#include "drops_manager.h"
#include "draw_utils.h"
#include "rooms/chambers.h"

// CD-i converted sprites
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

// when the pickups are first drawn, save the clean background area 
// underneath them. we'll be making them part of the clean background
// instead of other ports where the pickups are drawn every frames 
// because the CD-i can't draw all the sprites fast enough.
// When a pickup is, uh, picked up, it's erased and replaced with its
// background snapshot.

typedef struct
{
	dl_u8 valid;
	const Pickup* pickup;
	dl_u8 pixels[PICKUPS_NUM_SPRITE_WIDTH * PICKUPS_NUM_SPRITE_ROWS];
} PickupBackgroundSnapshot;

PickupBackgroundSnapshot g_pickupBackgroundSnapshots[NUM_PICKUPS_PER_ROOM];


#define NUM_REGEN_FRAMES 4
dl_u8 g_regenFrameCounter = 0;

const DoorInfo* g_queuedDoorInfo;
dl_u8 g_drawDoorFrameCount;

typedef void (*DrawRoomFunction)(GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(GameData* gameData, const Resources* resources);
void drawTitleScreen(GameData* gameData, const Resources* resources);
void drawTransition(GameData* gameData, const Resources* resources);
void drawWipeTransition(GameData* gameData, const Resources* resources);
void drawGetReadyScreen(GameData* gameData, const Resources* resources);

//void GameRunner_ChangedRoomCallback(const GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);
void GameRunner_TransitionDone(const GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

void Player_TriggerPickupErase(const Pickup* pickup);
void Player_TriggerDoorIsVisibleDrawDoor(const DoorInfo* doorInfo);
void savePickupBackgrounds(const PlayerData* playerData, dl_u8 roomNumber, const dl_u8* buffer);
void restorePickupBackground(const Pickup* pickup);

void GameRunner_Init(GameData* gameData, const Resources* resources)
{
	dl_u32 cursorSpriteRaw;

	//log("GameRunner_Init start\n");

	g_cleanBackground = (dl_u8*)malloc(FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

	cursorSpriteRaw = 0xffffffff;

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
	buildRegenSprites(&regenSprite, resources->sprites_player, PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_ROWS, NUM_REGEN_FRAMES);
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

	//Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;
	Game_TransitionDone = GameRunner_TransitionDone;

	Player_TriggerPickup = Player_TriggerPickupErase;
	Player_TriggerDoorIsVisible = Player_TriggerDoorIsVisibleDrawDoor;

	g_queuedDoorInfo = NULL;
	g_drawDoorFrameCount = 0;

	//log("GameRunner_Init 1\n");

	initEraseList();

	//log("GameRunner_Init 2\n");

	Game_Init(gameData, resources);

	//log("GameRunner_Init end\n");
}

void drawStaticText(const GameData* gameData, 
					const Resources* resources,
					dl_u8* cleanBackground)
{
	if (!gameData->currentPlayerData->playerNumber)
		drawUIText_256(resources->text_pl1, _1BPP_LOCATION_TO_256(PLAYERLIVES_TEXT_DRAW_LOCATION), &characterFont, cleanBackground);
	else
		drawUIText_256(resources->text_pl2, _1BPP_LOCATION_TO_256(PLAYERLIVES_TEXT_DRAW_LOCATION), &characterFont, cleanBackground);
	
	drawUIText_256(resources->text_chamber, _1BPP_LOCATION_TO_256(CHAMBER_TEXT_DRAW_LOCATION), &characterFont, cleanBackground);
	drawUIText_256(gameData->string_roomNumber, _1BPP_LOCATION_TO_256(CHAMBER_NUMBER_TEXT_DRAW_LOCATION), &characterFont, cleanBackground);
}

/*
void GameRunner_ChangedRoomCallback(const GameData* gameData, 
									dl_u8 roomNumber, 
									dl_s8 transitionType)
{
}
*/

void GameRunner_Update(GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(GameData* gameData, const Resources* resources)
{
	processEraseList();

	m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);
}

void drawDrops(const GameData* gameData)
{
    // draw drops
    const Drop* dropsRunner = gameData->dropData.drops;
	int loop;

    for (loop = 0; loop < NUM_DROPS; loop++)
    {
        if ((dl_s8)dropsRunner->wiggleTimer < 0 || // wiggling
            dropsRunner->wiggleTimer > 1)   // falling
        {
			drawSpriteDrop_384((dropsRunner->x << 1),
						   (dropsRunner->y >> 8),
						   0,
						   &dropsSprite);
        }

        dropsRunner++;
    }
}

dl_u8 eraseRegenSprite = 0;

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = PLAYERLIVES_ICON_Y;
	dl_u8 loop;

    for (loop = 0; loop < playerData->lives; loop++)
	{
        drawSprite16Opaque_384(x << 1,
					   y,
					   playerData->currentSpriteNumber,
					   &playerIconSprite);

		x += PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
        drawSprite16Opaque_384(x << 1,
                       y,
					   g_regenFrameCounter + (playerData->facingDirection ? 0 : NUM_REGEN_FRAMES),
					   &playerIconSpriteRegen);

		eraseRegenSprite = 2;
    }
	else if (eraseRegenSprite)
	{
		eraseRegenSprite--;
		addToEraseList(x << 1,
					   y,
					   playerIconSpriteRegen.frameHeight,
					   eraseSprite16);
	}
}

void drawAppearingDoor(const DoorInfo* doorInfo)
{
	int xPosition = doorInfo->x;
	// adjust the door position, as per the original game.
	if (xPosition > 40) 
		xPosition += 7;
	else
		xPosition -= 4;

	drawSprite16_384(xPosition << 1,
					doorInfo->y,
					0,
					&doorSprite,
					 FALSE);
}

void drawChamber(GameData* gameData, const Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;
	dl_u16 playerX = (playerData->x >> 8) << 1;
	dl_u16 playerY = (playerData->y >> 8);

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

	const Pickup* pickup;
	const Pickup* pickups;
    int loop;

	const BallData* ballData;
	const BirdData* birdData;

    int roomNumber;


	// draw queued door
	if (g_queuedDoorInfo && !g_drawDoorFrameCount)
	{
		drawAppearingDoor(g_queuedDoorInfo);
		g_queuedDoorInfo = NULL;
	}
	else
	{
		g_drawDoorFrameCount--;
	}

	drawDrops(gameData);

    // draw ball
    if (gameData->ballData.enabled)
    {
        ballData = &gameData->ballData;

		drawSprite16_384((ballData->x >> 8) << 1,
					   ballData->y >> 8,
					   (dl_s8)ballData->fallStateCounter < 0,
					   &ballSprite,
						 TRUE);
    }

    // draw bird
    if (gameData->birdData.state && currentTimer == 0)
    {
        birdData = &gameData->birdData;

		drawSprite16_384((birdData->x >> 8) << 1,
					   birdData->y >> 8,
					   birdData->animationFrame,
					   &birdSprite,
						 TRUE);
    }


	// draw player
    switch (playerData->state)
    {
    case PLAYER_STATE_SPLAT:
        drawSprite24_384(playerX,
                       playerY + 7,
                       playerData->splatFrameNumber,
					   &splatSprite);
        break;
    case PLAYER_STATE_REGENERATION: 

		g_regenFrameCounter = (g_regenFrameCounter + 1) & 3;

        drawSprite16_384(playerX,
                       playerY,
                       g_regenFrameCounter + (playerData->facingDirection ? 0 : NUM_REGEN_FRAMES),
					   &regenSprite,
						 TRUE);
        break;
    default: 
        drawSprite16_384(playerX,
                       playerY,
                       playerData->currentSpriteNumber,
					   &playerSprite,
						 TRUE);
    }

    // draw text
	drawUIText_384(gameData->string_timer, _1BPP_LOCATION_TO_384(TIMER_DRAW_LOCATION), &characterFont);
	drawUIText_384(playerData->scoreString, _1BPP_LOCATION_TO_384(SCORE_DRAW_LOCATION), &characterFont);  

	drawUIPlayerLives(playerData);

	//log("%d, %d, %d\n", mousey, mousey, mousey);

}

void Player_TriggerPickupErase(const Pickup* pickup)
{
	restorePickupBackground(pickup);

	addToEraseList(pickup->x << 1,
				   pickup->y,
				   g_pickUpSprites[pickup->type]->frameHeight,
				   eraseSprite16);

	addToOtherEraseList(pickup->x << 1,
						pickup->y,
						g_pickUpSprites[pickup->type]->frameHeight,
						eraseSprite16);
}



void Player_TriggerDoorIsVisibleDrawDoor(const DoorInfo* doorInfo)
{
	drawAppearingDoor(doorInfo);

	g_queuedDoorInfo = doorInfo;
	g_drawDoorFrameCount = 1;
}

void drawTitleScreen(GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);

	drawSprite8_384(gameData->numPlayers == 1 ? 32 : 128,
				   123,
				   0,
				   &cursorSprite);
}

void drawDoors(const PlayerData* playerData, 
			   const Resources* resources, 
			   dl_u8 roomNumber,
			   dl_u8* buffer)
{
	const DoorInfoData* doorInfoData;
	const DoorInfo* doorInfoRunner;
	dl_u8 loop;

	// draw doors
	doorInfoData = &resources->roomResources[roomNumber].doorInfoData;
	doorInfoRunner = doorInfoData->doorInfos;

	for (loop = 0; loop < doorInfoData->drawInfosCount; loop++)
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

			drawSprite16x16_256(xPosition << 1,
						doorInfoRunner->y,
						0,
						&doorSprite,
							buffer);
		}

		doorInfoRunner++;
	}
}

void savePickupBackgrounds(const PlayerData* playerData,
                            dl_u8 roomNumber,
                            const dl_u8* buffer)
{
	const Pickup* pickups = playerData->gamePickups[roomNumber];
	dl_u8 loop;

	for (loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		if ((pickups->state & playerData->playerMask))
		{
			const dl_u8* src = buffer + (pickups->x << 1) + (pickups->y * FRAMEBUFFER_WIDTH);
			dl_u8* dst = g_pickupBackgroundSnapshots[loop].pixels;
			dl_u8 row;

			g_pickupBackgroundSnapshots[loop].valid = 1;
			g_pickupBackgroundSnapshots[loop].pickup = pickups;

			for (row = 0; row < PICKUPS_NUM_SPRITE_ROWS; row++)
			{
				memcpy(dst, src, PICKUPS_NUM_SPRITE_WIDTH);
				src += FRAMEBUFFER_WIDTH;
				dst += PICKUPS_NUM_SPRITE_WIDTH;
			}
		}
		else
		{
			g_pickupBackgroundSnapshots[loop].valid = 0;
		}

		pickups++;
	}
}

void restorePickupBackground(const Pickup* pickup)
{
	dl_u8 loop;

	for (loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		if (g_pickupBackgroundSnapshots[loop].valid &&
			g_pickupBackgroundSnapshots[loop].pickup == pickup)
		{
			const dl_u8* src = g_pickupBackgroundSnapshots[loop].pixels;
			dl_u8* dst = g_cleanBackground + (pickup->x << 1) + (pickup->y * FRAMEBUFFER_WIDTH);
			dl_u8 row;

			for (row = 0; row < PICKUPS_NUM_SPRITE_ROWS; row++)
			{
				memcpy(dst, src, PICKUPS_NUM_SPRITE_WIDTH);
				dst += FRAMEBUFFER_WIDTH;
				src += PICKUPS_NUM_SPRITE_WIDTH;
			}

			g_pickupBackgroundSnapshots[loop].valid = 0; // defensive: no double-restore
			break;
		}
	}
}

void drawPickupsStatic(const PlayerData* playerData,
					   dl_u8 roomNumber,
					   dl_u8* buffer)
{
	const Pickup* pickups;
	dl_u8 loop;

    pickups = playerData->gamePickups[roomNumber];
    for (loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
    {
		if ((pickups->state & playerData->playerMask))
		{
			drawSprite16x10_256(pickups->x << 1,
					     pickups->y,
					     0,
					     g_pickUpSprites[pickups->type],
					     buffer);
        }

        pickups++;
    }
}

void drawCleanBackground(const GameData* gameData, 
						 const Resources* resources)
{
	dl_u16 loop;
	dl_u16 destinationOffset;
	dl_u8* videoBufferRunner;
	dl_u8* cleanBackgroundRunner;
	dl_u32 offset = SCREEN_OFFSET_X + (SCREEN_OFFSET_Y * SCREEN_WIDTH);

	//log("drawCleanBackground 1\n");

	convert1bppImageTo8bppCrtEffect(gameData->cleanBackground, 
										  g_cleanBackground,
										  FRAMEBUFFER_WIDTH,
										  FRAMEBUFFER_HEIGHT,
										  FRAMEBUFFER_WIDTH,
										  4);

	//log("%d, %d, %d\n", 0, gameData->transitionRoomNumber, 0);

	if (gameData->transitionRoomNumber < NUM_ROOMS)
	{
		drawStaticText(gameData, resources, g_cleanBackground);
		drawDoors(gameData->playerData, resources, gameData->transitionRoomNumber, g_cleanBackground);
		savePickupBackgrounds(gameData->playerData, gameData->transitionRoomNumber, g_cleanBackground);
		drawPickupsStatic(gameData->playerData, gameData->transitionRoomNumber, g_cleanBackground);
	}

	//log("drawCleanBackground 2\n");
	videoBufferRunner = BUFFER_START + offset;
	cleanBackgroundRunner = g_cleanBackground;
	for (loop = 0; loop < FRAMEBUFFER_HEIGHT; loop++)
	{
		memcpy(videoBufferRunner, cleanBackgroundRunner, FRAMEBUFFER_WIDTH);
		videoBufferRunner += SCREEN_WIDTH;
		cleanBackgroundRunner += FRAMEBUFFER_WIDTH;
	}

	swapVideoBuffers();

	videoBufferRunner = BUFFER_START + offset;
	cleanBackgroundRunner = g_cleanBackground;
	for (loop = 0; loop < FRAMEBUFFER_HEIGHT; loop++)
	{
		memcpy(videoBufferRunner, cleanBackgroundRunner, FRAMEBUFFER_WIDTH);
		videoBufferRunner += SCREEN_WIDTH;
		cleanBackgroundRunner += FRAMEBUFFER_WIDTH;
	}

	//log("drawCleanBackground 3\n");
	swapVideoBuffers();

	initEraseList();

	log("drawCleanBackground end\n");
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
	int lineLoop;

	dl_u32 framebufferOffset;
	dl_u32 cleanBackgroundOffset;
	const dl_u16* cleanBackground16;
	dl_u16* _32XFramebuffer;

	dl_u32 loop;
	dl_u16 innerLoop;

	for (lineLoop = g_oldTransitionCurrentLine; lineLoop < gameData->transitionCurrentLine; lineLoop++)
	{
		framebufferOffset = (SCREEN_OFFSET_X + ((SCREEN_OFFSET_Y + lineLoop) * SCREEN_WIDTH)) >> 1;
		cleanBackgroundOffset = (lineLoop * FRAMEBUFFER_WIDTH) >> 1;

		cleanBackground16 = (const dl_u16*)g_cleanBackground + cleanBackgroundOffset;

		_32XFramebuffer = (dl_u16*)(BUFFER_START);
		_32XFramebuffer += framebufferOffset;

		for (loop = 0; loop < 6; loop++)
		{
			memcpy(_32XFramebuffer, cleanBackground16, 256);
			_32XFramebuffer += (32 * (SCREEN_WIDTH >> 1));
			cleanBackground16 += (32 * 128);
		}
	}

	if (gameData->transitionCurrentLine == 32)
		return;

	framebufferOffset = (SCREEN_OFFSET_X + ((SCREEN_OFFSET_Y + gameData->transitionCurrentLine) * SCREEN_WIDTH)) >> 1;
	_32XFramebuffer = (dl_u16*)(BUFFER_START);
	_32XFramebuffer += framebufferOffset;

	for (loop = 0; loop < 6; loop++)
	{
		memset(_32XFramebuffer, 0x01, 256);
		_32XFramebuffer += (32 * (SCREEN_WIDTH >> 1));
	}
}

void drawWipeTransition(GameData* gameData, const Resources* resources)
{
	dl_u32 counter;

	if (gameData->transitionInitialDelay == 29)
    {
		g_oldTransitionCurrentLine = 0;

		convert1bppImageTo8bppCrtEffect(gameData->cleanBackground, 
											  g_cleanBackground,
											  FRAMEBUFFER_WIDTH,
											  FRAMEBUFFER_HEIGHT,
											  FRAMEBUFFER_WIDTH,
											  4);

		//log("%d, %d, %d\n", 0, gameData->transitionRoomNumber, 0);

		if (gameData->transitionRoomNumber < NUM_ROOMS)
		{
			drawStaticText(gameData, resources, g_cleanBackground);
			drawDoors(gameData->playerData, resources, gameData->transitionRoomNumber, g_cleanBackground);
			savePickupBackgrounds(gameData->playerData, gameData->transitionRoomNumber, g_cleanBackground);
			drawPickupsStatic(gameData->playerData, gameData->transitionRoomNumber, g_cleanBackground);
		}
	}

	if (gameData->transitionInitialDelay)
		return;

	drawTransitionLines(gameData);
	swapVideoBuffers();
	drawTransitionLines(gameData);
	
	// wait counter
	counter = 4;
	while (counter) 
	{
		counter--;
		swapVideoBuffers();
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
		swapVideoBuffers();
		drawTransitionLines(gameData);
		swapVideoBuffers();
	}
}

void drawGetReadyScreen(GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
