#include <stdio.h>
#include "SMSlib.h"

#include "base_types.h"
#include "game_data.h"
#include "game.h"
#include "custom_background_types.h"
#include "chambers.h"
#include "rooms/titlescreen.h"
#include "dl_sound.h"
#include "string.h"
#include "joystick_data.h"
#include "bird.h"
#include "drops_manager.h"
#include "resources.h"


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

//dl_u8 tileMapBuffer[32 * 16];

dl_u8 g_transitionDirection;

typedef void (*DrawRoomFunction)(void);
DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL];

void drawChamber(void);
void drawTitleScreen(void);
void drawTransition(void);
void drawWipeTransition(void);
void drawGetReadyScreen(void);

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{
	dl_u8* src = (dl_u8*)source;

	while (count--)
	{
		*src = value;
		src++;
	}
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	(void)soundIndex;
	(void)loop;
}

void Sound_Stop(dl_u8 soundIndex)
{
	(void)soundIndex;
}

void Sound_StopAll(void)
{
}


#define TRUE 1
#define FALSE 0

const dl_u8 downlandPalette[] = 
{
	0x00, 0x30, 0x0b, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const dl_u8 blackPalette[] = 
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

dl_u16 tileText[22];

void drawTileText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 tilex = (xyLocation & 31);
    dl_u16 tiley = (xyLocation >> 8);
	dl_u16* tileTextRunner = tileText;

    // for each character
    while (*text != 0xff)
    {
		*tileTextRunner = *text + 195; // text tiles offset in vdp
		tileTextRunner++;
		text++;
    }

	SMS_loadTileMap(tilex, tiley, tileText, (tileTextRunner - tileText) << 1);
}

void chamber_draw(dl_u8 roomNumber)
{
	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[roomNumber].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;
	SMS_loadTileMap(0, 0, (dl_u8*)backgroundData->tileMap, 32 * 24 * 2);

	drawTileText(res_string_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
	drawTileText(res_string_chamber, CHAMBER_TEXT_DRAW_LOCATION);
	drawTileText(gameData_string_roomNumber, CHAMBER_NUMBER_TEXT_DRAW_LOCATION);

	PlayerData* playerData = gameData_playerData;
	convertScoreToString(playerData->score, playerData->scoreString);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];
	convertTimerToString(currentTimer,
						 gameData_string_timer);
	drawTileText(gameData_string_timer, TIMER_DRAW_LOCATION);
}


void get_ready_room_draw(dl_u8 roomNumber)
{
	(void)roomNumber;

	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)getReadyScreen_cleanBackground;
	SMS_loadTileMap(0, 0, (dl_u8*)backgroundData->tileMap, 32 * 24 * 2);

	// get ready text
	const dl_u8* getReadyString = gameData_currentPlayerData->playerNumber == PLAYER_ONE ? res_string_getReadyPlayerOne : res_string_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);
}

void titleScreen_draw(dl_u8 roomNumber)
{
	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[roomNumber].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;
	SMS_loadTileMap(0, 0, (dl_u8*)backgroundData->tileMap, 32 * 24 * 2);

	drawTileText(res_string_downland, 0x03c9 + 64); // 0x07c9 original coco mem location
	drawTileText(res_string_writtenBy, 0x050a); // 0x090A original coco mem location
	drawTileText(res_string_michaelAichlmayer, 0x647); // 0x0A47 original coco mem location
	drawTileText(res_string_copyright1983, 0x789); // 0x0B89 original coco mem location
	drawTileText(res_string_spectralAssociates, 0x8c6); // 0x0CC6 original coco mem location
	drawTileText(res_string_licensedTo, 0xa0a); // 0x0E0A original coco mem location
	drawTileText(res_string_tandyCorporation, 0xb47); // 0x0F47 original coco mem location
	drawTileText(res_string_allRightsReserved, 0xc86); // 0x1086 original coco mem location
	drawTileText(res_string_onePlayer, 0xf05); // 0x1305 original coco mem location
	drawTileText(res_string_twoPlayer, 0xf11); // 0x1311 original coco mem location
	drawTileText(res_string_highScore, 0x118b); // 0x158B original coco mem location
	drawTileText(res_string_playerOne, 0x1406); // 0x1806 original coco mem location
	drawTileText(res_string_playerTwo, 0x1546); // 0x1946 original coco mem location

	convertScoreToString(gameData_playerData[PLAYER_ONE].score, gameData_playerData[PLAYER_ONE].scoreString);
	drawTileText(gameData_playerData[PLAYER_ONE].scoreString, TITLESCREEN_PLAYERONE_SCORE_LOCATION);

	convertScoreToString(gameData_playerData[PLAYER_TWO].score, gameData_playerData[PLAYER_TWO].scoreString);
	drawTileText(gameData_playerData[PLAYER_TWO].scoreString, TITLESCREEN_PLAYERTWO_SCORE_LOCATION);

	if (gameData_playerData[PLAYER_ONE].score > gameData_highScore)
		gameData_highScore = gameData_playerData[PLAYER_ONE].score;
	else if (gameData_playerData[PLAYER_TWO].score > gameData_highScore)
		gameData_highScore = gameData_playerData[PLAYER_TWO].score;

	convertScoreToString(gameData_highScore, gameData_string_highScore);

	drawTileText(gameData_string_highScore, TITLESCREEN_HIGHSCORE_LOCATION);
}

// moving the dropsDrawRunner to global memory makes drawDrops twice as fast
const Drop* dropsDrawRunner;

void drawDrops(void)
{
    // draw drops
    dropsDrawRunner = dropData_drops;

	int counter = NUM_DROPS;
	while (counter--)
    {
        if ((dl_s8)dropsDrawRunner->wiggleTimer < 0 || // wiggling
            dropsDrawRunner->wiggleTimer > 1)   // falling
        {
			SMS_addSprite((dropsDrawRunner->x << 1), 
						  (dropsDrawRunner->y >> 8), 
						  16);
        }

        dropsDrawRunner++;
    }
}

const dl_u8 pickUpSprites[] = { 8, 22, 18 };
dl_u8 pickupx;
dl_u8 pickupy;
const Pickup* pickups;
dl_u8 playerMask;

void drawPickups(void)
{
	// draw pickups
	int roomIndex = gameData_currentRoom->roomNumber;
	pickups = &gameData_currentPlayerData->gamePickups[roomIndex][0];
	playerMask = gameData_currentPlayerData->playerMask;

	for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		if ((pickups->state & playerMask))
		{
			const dl_u8 tileIndex = pickUpSprites[pickups->type];

			pickupx = (pickups->x) << 1;
			pickupy = pickups->y;

			SMS_addTwoAdjoiningSprites(pickupx, pickupy, tileIndex);
			SMS_addTwoAdjoiningSprites(pickupx, pickupy + 8, tileIndex + 2);
		}

		pickups++;
	}
}

void drawDoors(void)
{
	// draw doors
    dl_u8 roomNumber = gameData_currentRoom->roomNumber;
	const DoorInfoData* doorInfoData = &res_roomResources[roomNumber].doorInfoData;
	const DoorInfo* doorInfoRunner = doorInfoData->doorInfos;

	const dl_u8* doorStateData = gameData_currentPlayerData->doorStateData;
	dl_u8 playerMask = gameData_currentPlayerData->playerMask;

	for (dl_u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
        if ((doorStateData[doorInfoRunner->globalDoorIndex] & playerMask) &&
			doorInfoRunner->x != 0xff)
		{
			dl_u8 xPosition = (dl_u8)(doorInfoRunner->x << 1);

			// adjust the door position, as per the original game.
			xPosition += (xPosition > 80 ? 14 : -8);

			SMS_addTwoAdjoiningSprites(xPosition, doorInfoRunner->y, 12);
			SMS_addTwoAdjoiningSprites(xPosition, doorInfoRunner->y + 8, 14);
		}

		doorInfoRunner++;
	}
}

dl_u8 g_playerTileIndex;

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X << 1;
	dl_u8 y = PLAYERLIVES_ICON_Y;

	dl_u8 tileIndex = 80 + (playerData->currentSpriteNumber << 1);

	dl_u8 count = playerData->lives;
	while (count--)
	{
		SMS_addTwoAdjoiningSprites(x, y, tileIndex);

		x += (PLAYERLIVES_ICON_SPACING << 1);
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

		SMS_addTwoAdjoiningSprites(x, y, tileIndex);
    }
}

extern volatile unsigned int KeysStatus;
extern volatile unsigned int PreviousKeysStatus;

void updateControls(dl_u8 controllerIndex)
{
	UNUSED(controllerIndex); // support 2 player

	//dl_u16 state = JOY_readJoypad(controllerIndex);

    // Check D-Pad
    dl_u8 leftDown = (KeysStatus & PORT_A_KEY_LEFT) != 0;
    dl_u8 rightDown = (KeysStatus & PORT_A_KEY_RIGHT) != 0;
    dl_u8 upDown = (KeysStatus & PORT_A_KEY_UP) != 0;
    dl_u8 downDown = (KeysStatus & PORT_A_KEY_DOWN) != 0;
    dl_u8 jumpDown = (KeysStatus & PORT_A_KEY_1) != 0;
    //dl_u8 startDown = FALSE;//(KeysStatus & PORT_A_KEY_2) != 0;

    joystickState_leftPressed = (!joystickState_leftDown) & leftDown;
    joystickState_rightPressed = (!joystickState_rightDown) & rightDown;
    //joystickState_upPressed = (!joystickState_upDown) & upDown;
    //joystickState_downPressed =  (!joystickState_downDown) & downDown;
    joystickState_jumpPressed =  (!joystickState_jumpDown) & jumpDown;
    //joystickState_startPressed = (!joystickState_startDown) & startDown;

    //joystickState_leftReleased = joystickState_leftDown & (!leftDown);
    //joystickState_rightReleased = joystickState_rightDown & (!rightDown);
    //joystickState_upReleased = joystickState_upDown & (!upDown);
    //joystickState_downReleased =  joystickState_downDown & (!downDown);
    //joystickState_jumpReleased =  joystickState_jumpDown & (!jumpDown);
    //joystickState_startReleased = joystickState_startPressed & (!startDown);

    joystickState_leftDown = leftDown;
    joystickState_rightDown = rightDown;
    joystickState_upDown = upDown;
    joystickState_downDown = downDown;
    joystickState_jumpDown = jumpDown;
    //joystickState_startDown = startDown;

#ifdef DEV_MODE
    dl_u8 debugStateDown = (KeysStatus & PORT_A_KEY_2) != 0;

    joystickState_debugStatePressed = !joystickState_debugStateDown & debugStateDown;
    joystickState_debugStateDown = debugStateDown;
#endif
}



void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType);

void main(void)
{
	/* Clear VRAM */
	SMS_VRAMmemsetW(0, 0x0000, 16384);
	//SMS_setSpriteMode(SPRITEMODE_TALL);

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

	Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;

    Game_Init();

	dl_u8 controllerIndex = 0;

	for(;;) 
	{ 
        if (gameData_currentPlayerData != NULL)
        {
            controllerIndex = gameData_currentPlayerData->playerNumber;
        }

		updateControls(controllerIndex);

		/*
		if (joystickState_startPressed)
		{
			gameData_paused = !gameData_paused;

			if (gameData_paused)
				Sound_StopAll();
		}
		*/

		// Game Loop
		SMS_initSprites();

		if (!gameData_paused)
		{
			Game_Update();
		}

		m_drawRoomFunctions[gameData_currentRoom->roomNumber]();

		// VBLANK
		SMS_waitForVBlank ();

		//extern unsigned char SpriteNextFree;
		//SMS_debugPrintf("sprites: %d\n", SpriteNextFree);

		UNSAFE_SMS_copySpritestoSAT();
	}
}


void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType)
{
	UNUSED(roomNumber);

	//SMS_debugPrintf("GameRunner_ChangedRoomCallback\n");

	SMS_waitForVBlank();
	SMS_initSprites();
	SMS_copySpritestoSAT();

	//SMS_debugPrintf("transitionType: %d\n", transitionType);
	if (transitionType < 0)
	{
		//SMS_debugPrintf("downland palette 2\n");
		SMS_loadBGPalette(downlandPalette);
		SMS_loadSpritePalette(downlandPalette);
	}
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"pw","basicsmsproject","A basic SMS example project with devkitSMS");

dl_u8 tickTock;

void drawChamber(void)
{
	PlayerData* playerData = gameData_currentPlayerData;

	dl_u8 playerX = (playerData->x >> 8) << 1;
	dl_u8 playerY = (playerData->y >> 8);

	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];



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

		if (!gameData_paused)
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

	drawDoors();

	tickTock = !tickTock;

	if (tickTock)
	{
		Ball_Draw();
		Bird_Draw(currentTimer);
		drawPickups();
		drawDrops();
	}
	else
	{
		drawDrops();
		drawPickups();
		Bird_Draw(currentTimer);
		Ball_Draw();
	}

	drawTileText(gameData_string_timer, TIMER_DRAW_LOCATION);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);

	drawUIPlayerLives(playerData);

}

void drawTitleScreen(void)
{
	drawDrops();

	dl_u8 x = gameData_numPlayers == 1 ? 32 : 128;

	SMS_addSprite(x, 123, 160);
}

void drawTransition(void)
{
}

void drawWipeTransition(void)
{
}

void drawGetReadyScreen(void)
{
	drawDrops();
}

#define INITIAL_TRANSITION_DELAY 30

void transition_init(const Room* targetRoom)
{
	SMS_waitForVBlank();
	SMS_initSprites();
	SMS_copySpritestoSAT();
	SMS_VRAMmemset(XYtoADDR((0),(0)), 0, 32 * 24 * 2);

	if (gameData_transitionRoomNumber != GET_READY_ROOM_INDEX)
		SMS_mapROMBank(CHAMBER_BANK_START + gameData_transitionRoomNumber);
	else 
		SMS_mapROMBank(CHAMBER_BANK_START + TITLESCREEN_ROOM_INDEX);

	////SMS_debugPrintf("black palette\n");
	//SMS_loadBGPalette(blackPalette);
	//SMS_loadSpritePalette(blackPalette);

	// init the clean background with the target room. 
	// it'll be revealed at the end of the transition.
	targetRoom->draw(gameData_transitionRoomNumber);

	// setup screen transition
	gameData_transitionInitialDelay = INITIAL_TRANSITION_DELAY;

	////SMS_debugPrintf("transition_init\n");
}

void transition_update(Room* room)
{
	UNUSED(room);

	// wait to draw anything until the delay is over
	if (gameData_transitionInitialDelay)
	{
		gameData_transitionInitialDelay--;
		////SMS_debugPrintf("transition_update delay\n");
		return;
	}

	////SMS_debugPrintf("transition_update enter game room\n");
	Game_EnterRoom(gameData_transitionRoomNumber);
}

void wipe_transition_init(const Room* targetRoom)
{
	UNUSED(targetRoom);

	SMS_waitForVBlank();
	SMS_initSprites();
	SMS_copySpritestoSAT();
	SMS_VRAMmemset(XYtoADDR((0),(0)), 0, 32 * 24 * 2);

	if (gameData_transitionRoomNumber != GET_READY_ROOM_INDEX)
		SMS_mapROMBank(CHAMBER_BANK_START + gameData_transitionRoomNumber);
	else 
		SMS_mapROMBank(CHAMBER_BANK_START + TITLESCREEN_ROOM_INDEX);

	// setup screen transition
	gameData_transitionInitialDelay = INITIAL_TRANSITION_DELAY;
	gameData_transitionCurrentLine = 0;
	gameData_transitionFrameDelay = 0;

	const PlayerData* playerData = gameData_currentPlayerData;
	g_transitionDirection = playerData->lastDoor->xLocationInNextRoom < 50;

	//SMS_debugPrintf("transition_init\n");
}

void wipe_transition_update(Room* room)
{
	UNUSED(room);

	// wait to draw anything until the delay is over
	if (gameData_transitionInitialDelay)
	{
		gameData_transitionInitialDelay--;
		//SMS_debugPrintf("transition_update delay\n");
		return;
	}

	// only update every other frame to simulate the 
	// original game.
	gameData_transitionFrameDelay = !gameData_transitionFrameDelay;

	if (gameData_transitionFrameDelay)
		return;

	// play the transition sound effect at the start
	if (!gameData_transitionCurrentLine)
	{
		Sound_Play(SOUND_SCREEN_TRANSITION, FALSE);
	}

	dl_u8 currentColumn = g_transitionDirection ? gameData_transitionCurrentLine : 31 - gameData_transitionCurrentLine;
	dl_s8 offset = g_transitionDirection ? 1 : -1;
	dl_u16 flip = g_transitionDirection ? TILE_FLIPPED_X : 0;

	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[gameData_transitionRoomNumber].backgroundDrawData;
	for (dl_u8 loop = 0; loop < 24; loop++)
	{
		SMS_setTileatXY(currentColumn, 
						loop, 
						backgroundData->tileMap[currentColumn + (loop << 5)]);

		if (currentColumn > 0 && currentColumn < 30)
		{
			SMS_setTileatXY(currentColumn + offset, loop, 18 | flip);
		}
	}

	gameData_transitionCurrentLine++;

	// we're done
	if (gameData_transitionCurrentLine == 32)
	{
		Room* transitionRoom = g_rooms[gameData_transitionRoomNumber];
		transitionRoom->draw(gameData_transitionRoomNumber);

		Game_EnterRoom(gameData_transitionRoomNumber);

		if (Game_TransitionDone)
			Game_TransitionDone(gameData_transitionRoomNumber, WIPE_TRANSITION_ROOM_INDEX);
	}
}
