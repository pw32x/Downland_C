#include <stdio.h>
#include "SMSlib.h"

#include "base_types.h"
#include "game_types.h"
#include "custom_background_types.h"
#include "chambers.h"
#include "rooms/titlescreen.h"

#include "string.h"

#define VDP_ASSETS_BANK 2
#define CHAMBER_BANK_START 3
#define CHAMBER0_BANK 3
#define CHAMBER1_BANK 4
#define CHAMBER2_BANK 5
#define CHAMBER3_BANK 6
#define CHAMBER4_BANK 7
#define CHAMBER5_BANK 8
#define CHAMBER6_BANK 9
#define CHAMBER7_BANK 10
#define CHAMBER8_BANK 11
#define CHAMBER9_BANK 12
#define TITLE_SCREEN_BANK 13

dl_u8 g_regenSpriteIndex;
#define REGEN_NUM_FRAMES 5

extern const dl_u8 getReadyScreen_cleanBackground[6144];

typedef void (*DrawRoomFunction)(struct GameData* gameData, const Resources* resources);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(struct GameData* gameData, const Resources* resources);
void drawTitleScreen(struct GameData* gameData, const Resources* resources);
void drawTransition(struct GameData* gameData, const Resources* resources);
void drawWipeTransition(struct GameData* gameData, const Resources* resources);
void drawGetReadyScreen(struct GameData* gameData, const Resources* resources);

void* dl_alloc(dl_u32 size)
{
	return NULL;
}

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{

}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

void Sound_StopAll(void)
{
}


#define TRUE 1
#define FALSE 0

dl_u8 downlandPalette[] = 
{
	0x00, 0x30, 0x0b, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

dl_u8 blackPalette[] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

extern unsigned char const tileSet4bpp[6240];
extern unsigned char const characterFont4bpp[1248];
extern const dl_u16 chamber0_tileMap[];
extern const dl_u16 chamber1_tileMap[];

extern unsigned char const ball4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const bird4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const diamond4bpp[128];  // 4 tiles x 32 bytes
extern unsigned char const door4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const drop4bpp[64]; // 2 tiles x 32 bytes
extern unsigned char const key4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const moneyBag4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const player4bpp[1280]; // 40 tiles x 32 bytes
extern unsigned char const playerSplat4bpp[384];
extern unsigned char const playerLives4bpp[640]; // 20 tiles x 32 bytes
extern unsigned char const playerRegen4bpp[1280]; // 40 tiles x 32 bytes
extern unsigned char const playerLivesRegen4bpp[320]; // 10 tiles x 32 bytes
extern unsigned char const cursor4bpp[32]; // 1 tiles x 32 bytes

void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const Resources* resources,
					dl_u8* framebuffer)
{
}


void drawTileText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 tilex = ((xyLocation % 32) * 8) / 8;
    dl_u16 tiley = (xyLocation / 32) / 8;

    // for each character
    while (*text != 0xff)
    {
		//VDP_setTileMapXY(BG_B, characterFontSprite.vdpTileIndex + *text, tilex, tiley);

		SMS_setTileatXY(tilex, tiley, 195 + *text);

        text++;
        tilex++;
    }
}

void chamber_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	SMS_mapROMBank(CHAMBER_BANK_START + roomNumber);


	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)resources->roomResources[roomNumber].backgroundDrawData;
	gameData->cleanBackground = backgroundData->cleanBackground;
	SMS_loadTileMap(0, 0, backgroundData->tileMap, 32 * 24 * 2);

	drawTileText(resources->text_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
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


void get_ready_room_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	SMS_mapROMBank(CHAMBER_BANK_START + TITLESCREEN_ROOM_INDEX);

	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)resources->roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData;
	gameData->cleanBackground = getReadyScreen_cleanBackground;
	SMS_loadTileMap(0, 0, backgroundData->tileMap, 32 * 24 * 2);

	// get ready text
	const dl_u8* getReadyString = gameData->currentPlayerData->playerNumber == PLAYER_ONE ? resources->text_getReadyPlayerOne : resources->text_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);
}

void titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
	SMS_mapROMBank(CHAMBER_BANK_START + roomNumber);

	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)resources->roomResources[roomNumber].backgroundDrawData;
	gameData->cleanBackground = backgroundData->cleanBackground;
	SMS_loadTileMap(0, 0, backgroundData->tileMap, 32 * 24 * 2);

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



GameData gameData;
extern Resources resources;
dl_u8 framebuffer[FRAMEBUFFER_PITCH * FRAMEBUFFER_HEIGHT];


void drawDrops(const GameData* gameData)
{
    // draw drops
    const Drop* dropsRunner = gameData->dropData.drops;

    for (int loop = 0; loop < NUM_DROPS; loop++)
    {
        if ((dl_s8)dropsRunner->wiggleTimer < 0 || // wiggling
            dropsRunner->wiggleTimer > 1)   // falling
        {
			SMS_addSprite((dropsRunner->x << 1), 
						  (dropsRunner->y >> 8), 
						  16);
        }

        dropsRunner++;
    }
}

dl_u8 g_playerTileIndex;

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X;
	dl_u8 y = PLAYERLIVES_ICON_Y;

	dl_u8 tileIndex = 80 + (playerData->currentSpriteNumber << 1);

    for (dl_u8 loop = 0; loop < playerData->lives; loop++)
	{
		SMS_addTwoAdjoiningSprites(x << 1, y, tileIndex);

		x += PLAYERLIVES_ICON_SPACING;
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		if (playerData->facingDirection)
		{
			tileIndex = 140 + (g_regenSpriteIndex << 1);
		}
		else
		{
			tileIndex = 140 + ((g_regenSpriteIndex + REGEN_NUM_FRAMES) << 1);
		}

		SMS_addTwoAdjoiningSprites(x << 1, y, tileIndex);
    }
}

extern volatile unsigned int KeysStatus;
extern volatile unsigned int PreviousKeysStatus;

void updateControls(dl_u8 controllerIndex, JoystickState* joystickState)
{
	//dl_u16 state = JOY_readJoypad(controllerIndex);

    // Check D-Pad
    dl_u8 leftDown = (KeysStatus & PORT_A_KEY_LEFT) != 0;
    dl_u8 rightDown = (KeysStatus & PORT_A_KEY_RIGHT) != 0;
    dl_u8 upDown = (KeysStatus & PORT_A_KEY_UP) != 0;
    dl_u8 downDown = (KeysStatus & PORT_A_KEY_DOWN) != 0;
    dl_u8 jumpDown = (KeysStatus & PORT_A_KEY_1) != 0;
    dl_u8 startDown = FALSE;//(KeysStatus & PORT_A_KEY_2) != 0;

    joystickState->leftPressed = (!joystickState->leftDown) & leftDown;
    joystickState->rightPressed = (!joystickState->rightDown) & rightDown;
    joystickState->upPressed = (!joystickState->upDown) & upDown;
    joystickState->downPressed =  (!joystickState->downDown) & downDown;
    joystickState->jumpPressed =  (!joystickState->jumpDown) & jumpDown;
    joystickState->startPressed = (!joystickState->startDown) & startDown;

    joystickState->leftReleased = joystickState->leftDown & (!leftDown);
    joystickState->rightReleased = joystickState->rightDown & (!rightDown);
    joystickState->upReleased = joystickState->upDown & (!upDown);
    joystickState->downReleased =  joystickState->downDown & (!downDown);
    joystickState->jumpReleased =  joystickState->jumpDown & (!jumpDown);
    joystickState->startReleased = joystickState->startPressed & (!startDown);

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;
    joystickState->startDown = startDown;

#ifdef DEV_MODE
    dl_u8 debugStateDown = (KeysStatus & PORT_A_KEY_2) != 0;

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}

const dl_u8 pickUpSprites[] = { 8, 22, 18 };
const BallData* ballData = NULL;
const BirdData* birdData = NULL;

void main(void)
{
	/* Clear VRAM */
	SMS_VRAMmemsetW(0, 0x0000, 16384);

	/* Turn on the display */
	SMS_displayOn();
	SMS_loadBGPalette(blackPalette);
	SMS_loadSpritePalette(blackPalette);
	SMS_waitForVBlank ();

	SMS_initSprites();
	SMS_copySpritestoSAT();

	// load tiles for background
	SMS_mapROMBank(VDP_ASSETS_BANK);
	SMS_loadTiles(ball4bpp, 256, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(bird4bpp, 256 + 4, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(diamond4bpp, 256 + 8, 128);  // 4 tiles x 32 bytes
	SMS_loadTiles(door4bpp, 256 + 12, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(drop4bpp, 256 + 16, 64); // 2 tiles x 32 bytes
	SMS_loadTiles(key4bpp, 256 + 18, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(moneyBag4bpp, 256 + 22, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(player4bpp, 256 + 28, 1280); // 40 tiles x 32 bytes
	SMS_loadTiles(playerSplat4bpp, 256 + 68, 384); // 12 tiles x 32 bytes
	SMS_loadTiles(playerLives4bpp, 256 + 80, 640); // 20 tiles x 32 bytes
	SMS_loadTiles(playerRegen4bpp, 256 + 100, 1280); // 40 tiles x 32 bytes
	SMS_loadTiles(playerLivesRegen4bpp, 256 + 140, 640); // 20 tiles x 32 bytes
	SMS_loadTiles(cursor4bpp, 256 + 160, 32); // 1 tiles x 32 bytes
	

	g_regenSpriteIndex = 0;




	//g_pickUpSprites[0] = &diamondSprite;
	//g_pickUpSprites[1] = &moneyBagSprite;
	//g_pickUpSprites[2] = &keySprite

	SMS_loadTiles(tileSet4bpp, 0, 6240);
	SMS_loadTiles(characterFont4bpp, 195, 1248);

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

	memset((void*)gameData, 0, sizeof(gameData));

    Game_Init(&gameData, &resources, framebuffer, NULL /*cleanBackground*/);

	//chamber_draw(3, &gameData, &resources);

	SMS_waitForVBlank ();
	SMS_loadBGPalette(downlandPalette);
	SMS_loadSpritePalette(downlandPalette);


	ballData = &gameData.ballData;
	birdData = &gameData.birdData;


	dl_u8 controllerIndex = 0;

	for(;;) 
	{ 
        if (gameData.currentPlayerData != NULL)
        {
            controllerIndex = gameData.currentPlayerData->playerNumber;
        }

		updateControls(controllerIndex, &gameData.joystickState);

		if (gameData.joystickState.startPressed)
		{
			gameData.paused = !gameData.paused;

			if (gameData.paused)
				Sound_StopAll();
		}

		// Game Loop
		SMS_initSprites();

		if (!gameData.paused)
		{
			Game_Update(&gameData, &resources);
		}

		m_drawRoomFunctions[gameData.currentRoom->roomNumber](&gameData, &resources);

		// VBLANK
		SMS_waitForVBlank ();
		SMS_copySpritestoSAT();
	}
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"pw","basicsmsproject","A basic SMS example project with devkitSMS");



void drawChamber(struct GameData* gameData, const Resources* resources)
{
	PlayerData* playerData = gameData->currentPlayerData;

	dl_u8 playerX = (playerData->x >> 8) << 1;
	dl_u8 playerY = (playerData->y >> 8);

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

	drawDrops(gameData);
		
	// draw ball
	if (gameData->ballData.enabled)
	{


		//drawSprite((ballData->x >> 8) << 1,
		//		   ballData->y >> 8,
		//		   (dl_s8)ballData->fallStateCounter < 0,
		//		   &ballSprite);

		SMS_addTwoAdjoiningSprites((ballData->x >> 8) << 1, 
									ballData->y >> 8, 
									2 * ((dl_s8)ballData->fallStateCounter < 0));
	}

	// draw bird
	if (gameData->birdData.state && currentTimer == 0)
	{
		SMS_addTwoAdjoiningSprites((birdData->x >> 8) << 1,
									birdData->y >> 8,
									4 + (birdData->animationFrame << 1));
	}

	dl_u8 tileIndex;

	// draw player
	switch (playerData->state)
	{
	case PLAYER_STATE_SPLAT: 

		tileIndex = 68 + (playerData->splatFrameNumber * 6);

		SMS_addThreeAdjoiningSprites(playerX, playerY + 7, tileIndex);
		SMS_addThreeAdjoiningSprites(playerX, playerY + 15, tileIndex + 3);

		break;

	case PLAYER_STATE_REGENERATION: 

		if (!gameData->paused)
		{
			g_regenSpriteIndex++;
			if (g_regenSpriteIndex == REGEN_NUM_FRAMES - 1)
				g_regenSpriteIndex = 0;
		}

		if (playerData->facingDirection)
		{
			tileIndex = 100 + (g_regenSpriteIndex << 2);
		}
		else
		{
			tileIndex = 100 + ((g_regenSpriteIndex + REGEN_NUM_FRAMES) << 2);
		}

			
		SMS_addTwoAdjoiningSprites(playerX, playerY, tileIndex);
		SMS_addTwoAdjoiningSprites(playerX, playerY + 8, tileIndex + 2);

		break;

	default: 
		g_playerTileIndex = 28 + (playerData->currentSpriteNumber << 2);

		SMS_addTwoAdjoiningSprites(playerX, playerY, g_playerTileIndex);
		SMS_addTwoAdjoiningSprites(playerX, playerY + 8, g_playerTileIndex + 2);
	}

	// draw pickups
	int roomIndex = gameData->currentRoom->roomNumber;
	const Pickup* pickups = &playerData->gamePickups[roomIndex][0];
	for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		if ((pickups->state & playerData->playerMask))
		{
			const dl_u8 tileIndex = pickUpSprites[pickups->type];

			SMS_addTwoAdjoiningSprites((pickups->x) << 1, pickups->y, tileIndex);
			SMS_addTwoAdjoiningSprites((pickups->x) << 1, pickups->y + 8, tileIndex + 2);
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

//			drawSprite(xPosition << 1,
//					   doorInfoRunner->y,
//					   0,
//					   &doorSprite);

			SMS_addTwoAdjoiningSprites(xPosition << 1, doorInfoRunner->y, 12);
			SMS_addTwoAdjoiningSprites(xPosition << 1, doorInfoRunner->y + 8, 14);
		}

		doorInfoRunner++;
	}

	drawTileText(gameData->string_timer, TIMER_DRAW_LOCATION);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);

	drawUIPlayerLives(playerData);

}

void drawTitleScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);

	dl_u8 x = gameData->numPlayers == 1 ? 32 : 128;

	SMS_addSprite(x, 123, 160);
}

void drawTransition(struct GameData* gameData, const Resources* resources)
{

}

void drawWipeTransition(struct GameData* gameData, const Resources* resources)
{

}

void drawGetReadyScreen(struct GameData* gameData, const Resources* resources)
{
	drawDrops(gameData);
}
