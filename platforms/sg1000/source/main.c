
#include <stdio.h>
#include <SGlib.h>
#include <PSGLib.h>

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
#include "../generated/sprites/sprites.h"

#include "sounds.h"

/* function to print messages to the debug console of emulators */
void SMS_debugPrintf(const unsigned char *format, ...) __naked __preserves_regs(a,b,c,iyh,iyl);

const dl_u8 roomToBankIndex[] = 
{
    2, // chambers 0 to 9
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,  // title screen
	2,  // transition screen. it doesn't matter what the bank is.
	2,  // wipe transition screen. it doesn't matter what the bank is.
	7   // get ready screen
};

dl_u8 g_regenSpriteIndex;
#define REGEN_NUM_FRAMES 4

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

const dl_u8* sounds[SOUND_NUM_SOUNDS] = 
{
	jump_psg, // SOUND_JUMP				
	land_psg, // SOUND_LAND				
	transition_psg, // SOUND_SCREEN_TRANSITION	
	splat_psg, // SOUND_SPLAT				
	pickup_psg, // SOUND_PICKUP			
	run_psg, // SOUND_RUN				
	climb_up_psg, // SOUND_CLIMB_UP			
	climb_down_psg, // SOUND_CLIMB_DOWN		
};


dl_u8 isPlaying[SOUND_NUM_SOUNDS];

const dl_u8 isLooped[SOUND_NUM_SOUNDS] = 
{
	FALSE, // SOUND_JUMP				
	FALSE, // SOUND_LAND				
	FALSE, // SOUND_SCREEN_TRANSITION	
	FALSE, // SOUND_SPLAT				
	FALSE, // SOUND_PICKUP			
	TRUE, // SOUND_RUN				
	TRUE, // SOUND_CLIMB_UP			
	TRUE, // SOUND_CLIMB_DOWN		
};

const dl_u8 channels[SOUND_NUM_SOUNDS] = 
{
	SFX_CHANNEL2,
	SFX_CHANNELS2AND3,
	SFX_CHANNEL2,
	SFX_CHANNELS2AND3,
	SFX_CHANNEL2,
	SFX_CHANNELS2AND3,
	SFX_CHANNELS2AND3,
	SFX_CHANNELS2AND3,
};

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	if (loop)
	{
		if (isPlaying[soundIndex])
			return;

		PSGSFXPlayLoop(sounds[soundIndex], channels[soundIndex]);
		isPlaying[soundIndex] = TRUE;
	}
	else
		PSGSFXPlay(sounds[soundIndex], channels[soundIndex]);
}

void Sound_Stop(dl_u8 soundIndex)
{
	if (isLooped[soundIndex] && isPlaying[soundIndex])
		PSGSFXStop();

	isPlaying[soundIndex] = FALSE;
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

extern unsigned char const tileSet1bpp[1560];
extern unsigned char const characterFont1bpp[312];
extern unsigned char const drop1bpp[8];
extern unsigned char const cursor1bpp[8];

#define DROPS_TILE_INDEX 24

dl_u16 tileText[22];

void drawTileText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 tilex = (xyLocation & 31);
    dl_u16 tiley = (xyLocation >> 8);
	dl_u8* tileTextRunner = tileText;

    // for each character
    while (*text != 0xff)
    {
		*tileTextRunner = *text + 195; // text tiles offset in vdp
		tileTextRunner++;
		text++;
    }

	SG_loadTileMap(tilex, tiley, tileText, (tileTextRunner - tileText));
}

void chamber_draw(dl_u8 roomNumber)
{
	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[roomNumber].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;
	SG_loadTileMap(0, 0, backgroundData->tileMap, 32 * 24);

	if (!gameData_currentPlayerData->playerNumber)
		drawTileText(res_string_pl1, PLAYERLIVES_TEXT_DRAW_LOCATION);
	else
		drawTileText(res_string_pl2, PLAYERLIVES_TEXT_DRAW_LOCATION);

	drawTileText(res_string_chamber, CHAMBER_TEXT_DRAW_LOCATION);

	gameData_string_roomNumber[0] = roomNumber;
	drawTileText(gameData_string_roomNumber, CHAMBER_NUMBER_TEXT_DRAW_LOCATION);

	PlayerData* playerData = gameData_playerData;
	convertScoreToString(playerData->score, playerData->scoreString);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);

	drawTileText(gameData_string_timer, TIMER_DRAW_LOCATION);
}


void get_ready_room_draw(dl_u8 roomNumber)
{
	(void)roomNumber;

	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)getReadyScreen_cleanBackground;
	SG_loadTileMap(0, 0, backgroundData->tileMap, 32 * 24);

	// get ready text
	const dl_u8* getReadyString = gameData_currentPlayerData->playerNumber == PLAYER_ONE ? res_string_getReadyPlayerOne : res_string_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);
}

void titleScreen_draw(dl_u8 roomNumber)
{
	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[roomNumber].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;
	SG_loadTileMap(0, 0, backgroundData->tileMap, 32 * 24);

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

void updateScore(void)
{
	convertScoreToString(gameData_currentPlayerData->score, gameData_currentPlayerData->scoreString);
	drawTileText(gameData_currentPlayerData->scoreString, SCORE_DRAW_LOCATION);
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
			SG_addSprite((dropsDrawRunner->x << 1),  (dropsDrawRunner->y >> 8) - 1, DROPS_TILE_INDEX, SG_COLOR_WHITE);
        }

        dropsDrawRunner++;
    }
}

const dl_u8 pickUpSprites[] = { 16, 32, 28 }; // diamond, moneybag, key
const dl_u8 pickUpColors[] = { SG_COLOR_LIGHT_RED, SG_COLOR_LIGHT_GREEN, SG_COLOR_CYAN };
dl_u8 pickupx;
dl_u8 pickupy;
const Pickup* pickups;
dl_u8 playerMask;

void drawPickups(void)
{
	// draw pickups
	int roomIndex = gameData_currentRoom->roomNumber;
	pickups = &gameData_pickups[roomIndex][0];
	playerMask = gameData_currentPlayerData->playerMask;

	for (int loop = 0; loop < NUM_PICKUPS_PER_ROOM; loop++)
	{
		if ((pickups->state & playerMask))
		{
			const dl_u8 tileIndex = pickUpSprites[pickups->type];
			const dl_u8 pickupColor = pickUpColors[pickups->type];

			pickupx = (pickups->x) << 1;
			pickupy = pickups->y;

			//SG_addTwoAdjoiningSprites(pickupx, pickupy, tileIndex);
			SG_addSprite(pickupx, pickupy - 1, tileIndex, pickupColor);
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

	dl_u8 playerMask = gameData_currentPlayerData->playerMask;

	for (dl_u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
        if ((gameData_doorStateData[doorInfoRunner->globalDoorIndex] & playerMask) &&
			doorInfoRunner->x != 0xff)
		{
			dl_u8 xPosition = (dl_u8)(doorInfoRunner->x << 1);

			// adjust the door position, as per the original game.
			xPosition += (xPosition > 80 ? 14 : -8);

			//SG_addTwoAdjoiningSprites(xPosition, doorInfoRunner->y, 20);
			SG_addSprite(xPosition, doorInfoRunner->y - 1, 20, SG_COLOR_MEDIUM_RED);
		}

		doorInfoRunner++;
	}
}

dl_u8 g_playerTileIndex;

void drawUIPlayerLives(const PlayerData* playerData)
{


	dl_u8 x = PLAYERLIVES_ICON_X << 1;
	dl_u8 y = PLAYERLIVES_ICON_Y;

	dl_u8 tileIndex = 84 + (playerData->currentSpriteNumber << 2);

	dl_u8 count = playerData->lives;
	while (count--)
	{
		//SG_addTwoAdjoiningSprites(x, y, tileIndex);
		SG_addSprite(x, y - 1, tileIndex, SG_COLOR_WHITE);

		x += (PLAYERLIVES_ICON_SPACING << 1);
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		if (playerData->facingDirection)
		{
			tileIndex = 160 + (g_regenSpriteIndex << 2);
		}
		else
		{
			tileIndex = 160 + ((g_regenSpriteIndex + REGEN_NUM_FRAMES) << 2);
		}

		//SG_addTwoAdjoiningSprites(x, y, tileIndex);
		SG_addSprite(x, y - 1, tileIndex, SG_COLOR_WHITE);
    }
}

extern volatile unsigned int KeysStatus;
extern volatile unsigned int PreviousKeysStatus;

void updateControls(dl_u8 controllerIndex)
{
    dl_u8 leftDown;
    dl_u8 rightDown;
    dl_u8 upDown;
    dl_u8 downDown;
    dl_u8 jumpDown;

    // Check D-Pad
	if (!controllerIndex)
	{
		leftDown = (KeysStatus & PORT_A_KEY_LEFT) != 0;
		rightDown = (KeysStatus & PORT_A_KEY_RIGHT) != 0;
		upDown = (KeysStatus & PORT_A_KEY_UP) != 0;
		downDown = (KeysStatus & PORT_A_KEY_DOWN) != 0;
		jumpDown = ((KeysStatus & PORT_A_KEY_1) != 0) || ((KeysStatus & PORT_A_KEY_2) != 0);
	}
	else
	{
		leftDown = (KeysStatus & PORT_B_KEY_LEFT) != 0;
		rightDown = (KeysStatus & PORT_B_KEY_RIGHT) != 0;
		upDown = (KeysStatus & PORT_B_KEY_UP) != 0;
		downDown = (KeysStatus & PORT_B_KEY_DOWN) != 0;
		jumpDown = ((KeysStatus & PORT_B_KEY_1) != 0) || ((KeysStatus & PORT_B_KEY_2) != 0);
	}

    joystickState_leftPressed = (!joystickState_leftDown) & leftDown;
    joystickState_rightPressed = (!joystickState_rightDown) & rightDown;
    joystickState_jumpPressed =  (!joystickState_jumpDown) & jumpDown;

    joystickState_leftDown = leftDown;
    joystickState_rightDown = rightDown;
    joystickState_upDown = upDown;
    joystickState_downDown = downDown;
    joystickState_jumpDown = jumpDown;

#ifdef DEV_MODE
    dl_u8 debugStateDown = (KeysStatus & PORT_A_KEY_2) != 0;

    joystickState_debugStatePressed = !joystickState_debugStateDown & debugStateDown;
    joystickState_debugStateDown = debugStateDown;
#endif
}



void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType);

// unsigned int dst, const void *src, unsigned int size
// SMS_VRAMmemcpy((tilefrom)*32,(src),(size))
void load16x8SpriteTiles(const dl_u8* src, dl_u16 tilefrom, dl_u8 tileWidth)
{
	for (dl_u8 loop = 0; loop < tileWidth; loop++)
	{
		SG_loadSpritePatterns(src, tilefrom, 8);

		src += 8;
		tilefrom += 2;
	}
}

void load16x16SpriteTiles(const dl_u8* src, dl_u16 tilefrom, dl_u8 frames)
{
	for (dl_u8 loop = 0; loop < frames; loop++)
	{
		SG_loadSpritePatterns(src, tilefrom, 8); // 4 tiles x 8 bytes
		SG_loadSpritePatterns(src + 16, tilefrom + 1, 8); // 4 tiles x 8 bytes
		SG_loadSpritePatterns(src + 8, tilefrom + 2, 8); // 4 tiles x 8 bytes
		SG_loadSpritePatterns(src + 24, tilefrom + 3, 8); // 4 tiles x 8 bytes

		src += (8 * 4);
		tilefrom += 4;
	}
}

void PSGUpdate(void)
{
//	PSGFrame();
	PSGSFXFrame();
}

const dl_u8 blueTileColors[] = 
{
	0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50
};

void loadTilePatterns(dl_u8* tiles, dl_u16 tileIndex, dl_u16 size)
{
	SG_loadTilePatterns(tiles, tileIndex, size);
	SG_loadTilePatterns(tiles, tileIndex + 256, size);
	SG_loadTilePatterns(tiles, tileIndex + 512, size);
}

void setAllTileColors(dl_u8* tileColors)
{
	for (dl_u16 loop = 0; loop < 256; loop++)
	{
		SG_loadTileColours(tileColors, loop, 8);
		SG_loadTileColours(tileColors, loop + 256, 8);
		SG_loadTileColours(tileColors, loop + 512, 8);
	}
}

void setTileColor(dl_u8* tileColors, dl_u16 tileIndex)
{
	SG_loadTileColours(tileColors, tileIndex, 8);
	SG_loadTileColours(tileColors, tileIndex + 256, 8);
	SG_loadTileColours(tileColors, tileIndex + 512, 8);
}

void setupTileColors(void)
{

	setAllTileColors(blueTileColors);

	dl_u8 whiteTileColors[] = 
	{
		0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0
	};

	setTileColor(whiteTileColors, 13);
	setTileColor(whiteTileColors, 14);
	setTileColor(whiteTileColors, 38);
	setTileColor(whiteTileColors, 63);
	setTileColor(whiteTileColors, 67);
	setTileColor(whiteTileColors, 75);
	setTileColor(whiteTileColors, 172);
	setTileColor(whiteTileColors, 173);
	setTileColor(whiteTileColors, 174);
	setTileColor(whiteTileColors, 183);

	whiteTileColors[0] = 0x50;
	setTileColor(whiteTileColors, 139);
	setTileColor(whiteTileColors, 158);

	whiteTileColors[1] = 0x50;
	setTileColor(whiteTileColors, 133);

	whiteTileColors[2] = 0x50;
	setTileColor(whiteTileColors, 34);
	setTileColor(whiteTileColors, 135);
	setTileColor(whiteTileColors, 136);
	setTileColor(whiteTileColors, 137);
	setTileColor(whiteTileColors, 138);
	
	whiteTileColors[3] = 0x50;
	whiteTileColors[4] = 0x50;
	setTileColor(whiteTileColors, 72);
	setTileColor(whiteTileColors, 170);

	whiteTileColors[5] = 0x50;
	setTileColor(whiteTileColors, 5);
	setTileColor(whiteTileColors, 171);

	whiteTileColors[6] = 0x50;
	setTileColor(whiteTileColors, 186);
	setTileColor(whiteTileColors, 129);
	setTileColor(whiteTileColors, 131);
	setTileColor(whiteTileColors, 132);

	whiteTileColors[7] = 0x50;
	whiteTileColors[1] = 0xf0;
	setTileColor(whiteTileColors, 134);

	whiteTileColors[1] = 0x50;
	whiteTileColors[5] = 0xf0;
	setTileColor(whiteTileColors, 128);
}

void main(void)
{
	// Clear VRAM 
	SG_VRAMmemsetW(0, 0x0000, 16384);
	SG_setSpriteMode(SG_SPRITEMODE_LARGE);

	// Turn on the display
	SG_displayOn();
	SG_setBackdropColor(SG_COLOR_BLACK);
	//SG_loadBGPalette(blackPalette);
	//SG_loadSpritePalette(blackPalette);
	SG_waitForVBlank ();

	SG_initSprites();
	SG_finalizeSprites();
	SG_copySpritestoSAT();

	setupTileColors();

	// load tiles for background
	load16x8SpriteTiles(ball1bpp, 0, 4); // 4 tiles x 8 bytes
	load16x8SpriteTiles(bird1bpp, 8, 4); // 4 tiles x 8 bytes
	load16x16SpriteTiles(diamond1bpp, 16, 1);  // 4 tiles x 8 bytes
	load16x16SpriteTiles(door1bpp, 20, 1); // 4 tiles x 8 bytes
	SG_loadSpritePatterns(drop1bpp, 24, sizeof(drop1bpp)); // 1 tiles x 8 bytes
	load16x16SpriteTiles(key1bpp, 28, 1); // 4 tiles x 8 bytes
	load16x16SpriteTiles(moneyBag1bpp, 32, 1); // 4 tiles x 8 bytes
	
	load16x16SpriteTiles(player1bpp, 36, 10); // 40 tiles x 8 bytes
	load16x16SpriteTiles(playerSplat1bpp, 76, 2); // 12 tiles x 8 bytes
	
	load16x8SpriteTiles(playerLives1bpp, 84, 20); // 20 tiles x 8 bytes
	load16x16SpriteTiles(playerRegen1bpp, 124, 8); // 32 tiles x 8 bytes
	load16x8SpriteTiles(cursor1bpp, 156, 1); // 1 tiles x 8 bytes
	load16x8SpriteTiles(playerLivesRegen1bpp, 160, 16); // 16 tiles x 8 bytes
	
	g_regenSpriteIndex = 0;


	// load the background tiles in three different areas of vram
	loadTilePatterns(tileSet1bpp, 0, sizeof(tileSet1bpp));
	loadTilePatterns(characterFont1bpp, 195, sizeof(characterFont1bpp));

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

	SG_setFrameInterruptHandler(PSGUpdate);

	dl_u8 counter=0;

	for(;;) 
	{ 
		//SG_setBackdropColor(counter & 0xf);
		counter++;

        if (gameData_currentPlayerData != NULL)
        {
            controllerIndex = gameData_currentPlayerData->playerNumber;
        }

		updateControls(controllerIndex);

		// Game Loop
		SG_initSprites();

		if (!gameData_paused)
		{
			Game_Update();
		}

		m_drawRoomFunctions[gameData_currentRoom->roomNumber]();

		//SMS_debugPrintf("Test\n");

		//SG_setBackdropColor(SG_COLOR_BLACK);
		// VBLANK
		SG_finalizeSprites();
		SG_waitForVBlank ();

		//extern unsigned char SpriteNextFree;
		//SMS_debugPrintf("sprites: %d\n", SpriteNextFree);

		SG_copySpritestoSAT();
	}
}


void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType)
{
	UNUSED(roomNumber);

	//SMS_debugPrintf("GameRunner_ChangedRoomCallback\n");

	SG_waitForVBlank();
	SG_initSprites();
	SG_finalizeSprites();
	SG_copySpritestoSAT();

	//SMS_debugPrintf("transitionType: %d\n", transitionType);
	if (transitionType < 0)
	{
		//SMS_debugPrintf("downland palette 2\n");
		//SG_loadBGPalette(downlandPalette);
		//SG_loadSpritePalette(downlandPalette);
	}
}

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

		tileIndex = 76 + (playerData->splatFrameNumber * 4);

		SG_addSprite(playerX, playerY + 6, tileIndex, SG_COLOR_WHITE);
		//SG_addThreeAdjoiningSprites(playerX, playerY + 7, tileIndex);

		break;

	case PLAYER_STATE_REGENERATION: 

		if (!gameData_paused)
		{
			g_regenSpriteIndex++;
			if (g_regenSpriteIndex == REGEN_NUM_FRAMES)
				g_regenSpriteIndex = 0;
		}

		if (playerData->facingDirection)
		{
			tileIndex = 124 + (g_regenSpriteIndex << 2);
		}
		else
		{
			tileIndex = 124 + ((g_regenSpriteIndex + REGEN_NUM_FRAMES) << 2);
		}

		//SG_addTwoAdjoiningSprites(playerX, playerY, tileIndex);
		SG_addSprite(playerX, playerY - 1, tileIndex, SG_COLOR_WHITE);

		break;

	default: 
		g_playerTileIndex = 36 + (playerData->currentSpriteNumber << 2);

		SG_addSprite(playerX, playerY - 1, g_playerTileIndex, SG_COLOR_WHITE);

		//SG_addTwoAdjoiningSprites(playerX, playerY, g_playerTileIndex);
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

	SG_addSprite(x, 122, 156, SG_COLOR_WHITE);
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
	SG_waitForVBlank();
	SG_initSprites();
	SG_finalizeSprites();
	SG_copySpritestoSAT();

	//SG_VRAMmemset(XYtoADDR((0),(0)), 0, 32 * 24 * 2);

	SG_mapROMBank(roomToBankIndex[gameData_transitionRoomNumber]);

	////SMS_debugPrintf("black palette\n");
	//SG_loadBGPalette(blackPalette);
	//SG_loadSpritePalette(blackPalette);

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

	SG_waitForVBlank();
	SG_initSprites();
	SG_finalizeSprites();
	SG_copySpritestoSAT();
	//SG_VRAMmemset(XYtoADDR((0),(0)), 0, 32 * 24 * 2);

	SG_mapROMBank(roomToBankIndex[gameData_transitionRoomNumber]);

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
	dl_u16 flip = 0;//g_transitionDirection ? TILE_FLIPPED_X : 0;

	const SMSBackgroundData* backgroundData = (const SMSBackgroundData*)res_roomResources[gameData_transitionRoomNumber].backgroundDrawData;
	for (dl_u8 loop = 0; loop < 24; loop++)
	{
		SG_setTileatXY(currentColumn, 
						loop, 
						backgroundData->tileMap[currentColumn + (loop << 5)]);

		if (currentColumn > 0 && currentColumn < 30)
		{
			SG_setTileatXY(currentColumn + offset, loop, 18);
		}
	}

	gameData_transitionCurrentLine++;

	// we're done
	if (gameData_transitionCurrentLine == 32)
	{
		const Room* transitionRoom = g_rooms[gameData_transitionRoomNumber];
		transitionRoom->draw(gameData_transitionRoomNumber);

		Game_EnterRoom(gameData_transitionRoomNumber);
	}
}
