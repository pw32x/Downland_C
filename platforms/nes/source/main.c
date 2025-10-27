#include <stdio.h>
//#include "SMSlib.h"
//#include <PSGLib.h>
#include <neslib.h>
#include <mapper.h>


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

//#include "sounds.h"

const dl_u8 roomToBankIndex[] = 
{
    0, // chambers 0 to 9
    0,
    1,
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,  // title screen
	0,  // transition screen. it doesn't matter what the bank is.
	0,  // wipe transition screen. it doesn't matter what the bank is.
	5   // get ready screen
};

char metasprite[] = 
{
    0, 0, 0x02, 0, 
    8, 0, 0x04, 0,  
    128
};

#define SPRITE_TILE1 2
#define SPRITE_TILE2 6

inline void updateMetaSprite(dl_u8 startTile)
{
	metasprite[SPRITE_TILE1] = startTile;
	metasprite[SPRITE_TILE2] = startTile + 2;
}

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

/*
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
*/
void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	(void)soundIndex;
	(void)loop;
	/*
	if (loop)
	{
		if (isPlaying[soundIndex])
			return;

		PSGSFXPlayLoop(sounds[soundIndex], channels[soundIndex]);
		isPlaying[soundIndex] = TRUE;
	}
	else
		PSGSFXPlay(sounds[soundIndex], channels[soundIndex]);
		*/
}

void Sound_Stop(dl_u8 soundIndex)
{
	/*
	if (isLooped[soundIndex] && isPlaying[soundIndex])
		PSGSFXStop();

	isPlaying[soundIndex] = FALSE;
	*/
}

void Sound_StopAll(void)
{
}

void Ball_Draw(void)
{
	if (ballData_enabled)
	{
		/*
		SMS_addTwoAdjoiningSprites((ballData_x >> 8) << 1, 
									ballData_y >> 8, 
									((dl_s8)ballData_fallStateCounter < 0) << 2);
		*/

		updateMetaSprite(0x2a + (((dl_s8)ballData_fallStateCounter < 0) << 2));
		oam_meta_spr((ballData_x >> 8) << 1, ballData_y >> 8, metasprite);  
	}
}

#define BIRD_TILE_INDEX 0x32

void Bird_Draw(dl_u16 currentTimer)
{
	// draw bird
	if (birdData_state && currentTimer == 0)
	{
		/*
		SMS_addTwoAdjoiningSprites((birdData_x >> 8) << 1,
									birdData_y >> 8,
									BIRD_TILE_INDEX + (birdData_animationFrame << 2));
		*/
		updateMetaSprite(BIRD_TILE_INDEX + (birdData_animationFrame << 2));
		oam_meta_spr((birdData_x >> 8) << 1, birdData_y >> 8, metasprite);  

	}
}

#define TRUE 1
#define FALSE 0


dl_u16 tileText[22];

void drawTileText(const dl_u8* text, dl_u16 xyLocation)
{
    dl_u16 tilex = (xyLocation & 31);
    dl_u16 tiley = (xyLocation >> 8);
	dl_u16* tileTextRunner = tileText;

    // for each character
    while (*text != 0xff)
    {
		*tileTextRunner = *text + (256 + 195); // text tiles offset in vdp
		tileTextRunner++;
		text++;
    }


	 vram_write(tileText,sizeof((tileTextRunner - tileText)));

	/*
	SMS_loadTileMap(tilex, tiley, tileText, (tileTextRunner - tileText) << 1);
	*/
}

void chamber_draw(dl_u8 roomNumber)
{
	const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[roomNumber].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;

	/*
	SMS_loadTileMap(0, 0, (dl_u8*)backgroundData->tileMap, 32 * 24 * 2);
	*/

	//ppu_off(); // screen off
	vram_write((dl_u8*)backgroundData->tileMap, 32 * 24);
	//ppu_on_all(); //	turn on screen

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

	const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)getReadyScreen_cleanBackground;

	/*
	SMS_loadTileMap(0, 0, (dl_u8*)backgroundData->tileMap, 32 * 24 * 2);
	*/


	ppu_off(); // screen off
	vram_write((dl_u8*)backgroundData->tileMap, 32 * 24);
	ppu_on_all(); //	turn on screen

	// get ready text
	const dl_u8* getReadyString = gameData_currentPlayerData->playerNumber == PLAYER_ONE ? res_string_getReadyPlayerOne : res_string_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);
}

void titleScreen_draw(dl_u8 roomNumber)
{
	const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[roomNumber].backgroundDrawData;
	gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;

	/*
	SMS_loadTileMap(0, 0, (dl_u8*)backgroundData->tileMap, 32 * 24 * 2);
	*/


	//ppu_off(); // screen off
	vram_write((dl_u8*)backgroundData->tileMap, 32 * 24);
	//ppu_on_all(); //	turn on screen

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
			/*
			SMS_addSprite((dropsDrawRunner->x << 1), 
						  (dropsDrawRunner->y >> 8), 
						  24);
			*/

			oam_spr((dropsDrawRunner->x << 1), (dropsDrawRunner->y >> 8), 0, 0);
        }

        dropsDrawRunner++;
    }
}

const dl_u8 pickUpSprites[] = {  0x3e, 0x42, 0x3a };  // diamond, bag, key
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

			pickupx = (pickups->x) << 1;
			pickupy = pickups->y;

			/*
			SMS_addTwoAdjoiningSprites(pickupx, pickupy, tileIndex);
			*/

			updateMetaSprite(tileIndex);
			oam_meta_spr(pickupx, pickupy, metasprite);  
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

	const dl_u8* doorStateData = gameData_doorStateData;
	dl_u8 playerMask = gameData_currentPlayerData->playerMask;

	for (dl_u8 loop = 0; loop < doorInfoData->drawInfosCount; loop++)
	{
        //if ((doorStateData[doorInfoRunner->globalDoorIndex] & playerMask) &&
		//	doorInfoRunner->x != 0xff)
		{
			dl_u8 xPosition = (dl_u8)(doorInfoRunner->x << 1);

			// adjust the door position, as per the original game.
			xPosition += (xPosition > 80 ? 14 : -8);

			/*
			SMS_addTwoAdjoiningSprites(xPosition, doorInfoRunner->y, 20);
			*/

			updateMetaSprite(0x46);
			oam_meta_spr(xPosition, doorInfoRunner->y, metasprite);  
		}

		doorInfoRunner++;
	}
}

dl_u8 g_playerTileIndex;

void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X << 1;
	dl_u8 y = PLAYERLIVES_ICON_Y;

	dl_u8 tileIndex = 0x88 + (playerData->currentSpriteNumber << 2);

	dl_u8 count = playerData->lives;
	while (count--)
	{
		/*
		SMS_addTwoAdjoiningSprites(x, y, tileIndex);
		*/

		updateMetaSprite(tileIndex);
		oam_meta_spr(x, y, metasprite);  

		x += (PLAYERLIVES_ICON_SPACING << 1);
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		if (playerData->facingDirection)
		{
			tileIndex = 0x6c + (g_regenSpriteIndex << 2);
		}
		else
		{
			tileIndex = 0x6c + ((g_regenSpriteIndex + REGEN_NUM_FRAMES) << 2);
		}

		updateMetaSprite(tileIndex);
		oam_meta_spr(x, y, metasprite);  

		/*
		SMS_addTwoAdjoiningSprites(x, y, tileIndex);
		*/
    }
}

extern volatile unsigned int KeysStatus;
extern volatile unsigned int PreviousKeysStatus;

void updateControls(dl_u8 controllerIndex)
{
    dl_u8 leftDown = FALSE;
    dl_u8 rightDown = FALSE;
    dl_u8 upDown = FALSE;
    dl_u8 downDown = FALSE;
    dl_u8 jumpDown = FALSE;

	/*
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
	*/

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


void PSGUpdate(void)
{
	//PSGSFXFrame();
}

static const char palette[16] = 
{
    0x0f, 0x11, 0x27, 0x30 // black, gray, lt gray, white
};

int main(void)
{
	ppu_off(); // screen off
	vram_adr(NAMETABLE_A);
	//vram_write(chamber5_tileMap, 32 * 24);

	// load the palettes
	pal_bg(palette);
	pal_spr(palette);
	oam_size(1);
	bank_spr(0);
	bank_bg(1);
	ppu_on_all(); //	turn on screen

	/*
	// Clear VRAM

	SMS_VRAMmemsetW(0, 0x0000, 16384);
	SMS_setSpriteMode(SPRITEMODE_TALL);

	// Turn on the display
	SMS_displayOn();
	SMS_loadBGPalette(blackPalette);
	SMS_loadSpritePalette(blackPalette);
	SMS_waitForVBlank ();

	SMS_initSprites();
	SMS_copySpritestoSAT();
	*/

	g_regenSpriteIndex = 0;

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

	/*
	SMS_setFrameInterruptHandler(PSGUpdate);
	*/

  char y_position = 0x40; // all share same Y, which increases every frame
  char x_position = 0x88;
  char x_position2 = 0xa0;
  char x_position3 = 0xc0;

    const char metasprite2[] = 
    {
        0, 0, 0x02, 0, 
        8, 0, 0x04, 0,  
        128
    };

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
		/*
		SMS_initSprites();
		*/

		if (!gameData_paused)
		{
			Game_Update();
		}

		m_drawRoomFunctions[gameData_currentRoom->roomNumber]();

		// VBLANK
		/*
		SMS_waitForVBlank ();
		*/

		//extern unsigned char SpriteNextFree;
		//SMS_debugPrintf("sprites: %d\n", SpriteNextFree);

		/*
		UNSAFE_SMS_copySpritestoSAT();
		*/

		ppu_wait_nmi(); // wait till beginning of the frame
		// the sprites are pushed from a buffer to the OAM during nmi

		// clear all sprites from sprite buffer
		oam_clear();

		//oam_meta_spr(x_position3, y_position, metasprite2);    
	}

	return 0;
}


void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType)
{
	UNUSED(roomNumber);

	//SMS_debugPrintf("GameRunner_ChangedRoomCallback\n");

	/*
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
	*/
}

//SMS_EMBED_SEGA_ROM_HEADER(9999,0);
//SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"pw","Downland","Downland ported to the Sega Master System");

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

		tileIndex = 0x7c + (playerData->splatFrameNumber * 6);

		/*
		SMS_addThreeAdjoiningSprites(playerX, playerY + 7, tileIndex);
		*/

		updateMetaSprite(tileIndex);
		oam_meta_spr(playerX, playerY, metasprite);  

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
			tileIndex = 0x4c + (g_regenSpriteIndex << 2);
		}
		else
		{
			tileIndex = 0x4c + ((g_regenSpriteIndex + REGEN_NUM_FRAMES) << 2);
		}

		/*
		SMS_addTwoAdjoiningSprites(playerX, playerY, tileIndex);
		*/

		updateMetaSprite(tileIndex);
		oam_meta_spr(playerX, playerY, metasprite);  

		break;

	default: 
		g_playerTileIndex = 0x2 + (playerData->currentSpriteNumber << 2);

		/*
		SMS_addTwoAdjoiningSprites(playerX, playerY, g_playerTileIndex);
		*/


		updateMetaSprite(g_playerTileIndex);
		oam_meta_spr(playerX, playerY, metasprite);  
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

	/*
	SMS_addSprite(x, 123, 158);
	*/

	oam_spr(x, 123, 0x4a, 0);
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
    ppu_wait_nmi();
    oam_clear();
	set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);

	/*
	SMS_waitForVBlank();
	SMS_initSprites();
	SMS_copySpritestoSAT();
	SMS_VRAMmemset(XYtoADDR((0),(0)), 0, 32 * 24 * 2);

	SMS_mapROMBank(roomToBankIndex[gameData_transitionRoomNumber]);
	*/

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

    ppu_wait_nmi();
    oam_clear();
	set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);

	/*

	SMS_waitForVBlank();
	SMS_initSprites();
	SMS_copySpritestoSAT();
	SMS_VRAMmemset(XYtoADDR((0),(0)), 0, 32 * 24 * 2);

	SMS_mapROMBank(roomToBankIndex[gameData_transitionRoomNumber]);
	*/

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

	const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[gameData_transitionRoomNumber].backgroundDrawData;
	for (dl_u8 loop = 0; loop < 24; loop++)
	{
		/*
		SMS_setTileatXY(currentColumn, 
						loop, 
						backgroundData->tileMap[currentColumn + (loop << 5)]);
		*/
		if (currentColumn > 0 && currentColumn < 30)
		{
			/*
			SMS_setTileatXY(currentColumn + offset, loop, 18 | flip);
			*/
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
