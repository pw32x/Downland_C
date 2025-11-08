#include <stdio.h>
//#include "SMSlib.h"
//#include <PSGLib.h>
#include <neslib.h>
#include <nesdoug.h>
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

#define SCROLL_SPRITE_OFFSET 16

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

dl_u8 g_transitionDirection;

void drawChamber(void);
void drawTitleScreen(void);
void drawTransition(void);
void drawWipeTransition(void);
void drawGetReadyScreen(void);

typedef void (*DrawRoomFunction)(void);
const DrawRoomFunction m_drawRoomFunctions[NUM_ROOMS_AND_ALL] =
{
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawChamber,
    drawTitleScreen,
    drawTransition,
    drawWipeTransition,
    drawGetReadyScreen
};


void dl_memset(void* source, dl_u8 value, dl_u16 count)
{
	dl_u8* src = (dl_u8*)source;

	while (count--)
	{
		*src = value;
		src++;
	}
}

extern const char looping_effects_data[];
extern const char sound_effects_data[];

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


void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	if (loop)
	{
		if (isPlaying[soundIndex])
			return;

		music_play(soundIndex - SOUND_RUN);

		isPlaying[soundIndex] = TRUE;
	}
	else
	{
		sfx_play(soundIndex, 0);
	}
}

void Sound_Stop(dl_u8 soundIndex)
{
	(void)soundIndex;

	if (isLooped[soundIndex] && isPlaying[soundIndex])
		music_stop();

	isPlaying[soundIndex] = FALSE;
}

void Sound_StopAll(void)
{
	music_stop();
}

__attribute__((section(".prg_rom_5")))
void Ball_Draw(void)
{
	if (ballData_enabled)
	{
		updateMetaSprite(0x2a + (((dl_s8)ballData_fallStateCounter < 0) << 2));
		oam_meta_spr((ballData_x >> 8) << 1, (ballData_y >> 8) + SCROLL_SPRITE_OFFSET, metasprite);  
	}
}

#define BIRD_TILE_INDEX 0x32

__attribute__((section(".prg_rom_5")))
void Bird_Draw(dl_u16 currentTimer)
{
	// draw bird
	if (birdData_state && currentTimer == 0)
	{
		updateMetaSprite(BIRD_TILE_INDEX + (birdData_animationFrame << 2));
		oam_meta_spr((birdData_x >> 8) << 1, (birdData_y >> 8) + SCROLL_SPRITE_OFFSET, metasprite);  

	}
}

#define TRUE 1
#define FALSE 0


dl_u8 tileText[22];

__attribute__((section(".prg_rom_5")))
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

	multi_vram_buffer_horz(tileText, (tileTextRunner - tileText), NTADR_A(tilex, tiley));

}

__attribute__((section(".prg_rom_5")))
void chamber_draw(dl_u8 roomNumber)
{
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

__attribute__((section(".prg_rom_5")))
void get_ready_room_draw(dl_u8 roomNumber)
{
	(void)roomNumber;

	// we're reusing the title screen, so clear out the
	// title screen text.
	for (int loop = 3; loop < 22; loop++)
	{
		vram_adr(NTADR_A(4, loop));
		vram_fill(0, 24);
	}

	// get ready text
	const dl_u8* getReadyString = gameData_currentPlayerData->playerNumber == PLAYER_ONE ? res_string_getReadyPlayerOne : res_string_getReadyPlayerTwo;
	drawTileText(getReadyString, 0x0b66);

}

__attribute__((section(".prg_rom_5")))
void titleScreen_draw(dl_u8 roomNumber)
{
	(void)roomNumber;

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

	set_prg_bank(5);
	drawTileText(gameData_currentPlayerData->scoreString, SCORE_DRAW_LOCATION);
	set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);
}

// moving the dropsDrawRunner to global memory makes drawDrops twice as fast
const Drop* dropsDrawRunner;

__attribute__((section(".prg_rom_5")))
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
			oam_spr((dropsDrawRunner->x << 1), (dropsDrawRunner->y >> 8) + SCROLL_SPRITE_OFFSET, 0, 0);
        }

        dropsDrawRunner++;
    }
}

const dl_u8 pickUpSprites[] = {  0x3e, 0x42, 0x3a };  // diamond, bag, key
dl_u8 pickupx;
dl_u8 pickupy;
const Pickup* pickups;
dl_u8 playerMask;

__attribute__((section(".prg_rom_5")))
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

			updateMetaSprite(tileIndex);
			oam_meta_spr(pickupx, pickupy + SCROLL_SPRITE_OFFSET, metasprite);  
		}

		pickups++;
	}
}
__attribute__((noinline, section(".prg_rom_5")))
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
        if ((doorStateData[doorInfoRunner->globalDoorIndex] & playerMask) &&
			doorInfoRunner->x != 0xff)
		{
			dl_u8 xPosition = (dl_u8)(doorInfoRunner->x << 1);

			// adjust the door position, as per the original game.
			xPosition += (xPosition > 80 ? 14 : -8);

			updateMetaSprite(0x46);
			oam_meta_spr(xPosition, doorInfoRunner->y + SCROLL_SPRITE_OFFSET, metasprite);  
		}

		doorInfoRunner++;
	}
}

dl_u8 g_playerTileIndex;

__attribute__((section(".prg_rom_5")))
void drawUIPlayerLives(const PlayerData* playerData)
{
	dl_u8 x = PLAYERLIVES_ICON_X << 1;
	dl_u8 y = PLAYERLIVES_ICON_Y;

	dl_u8 tileIndex = 0x98 + (playerData->currentSpriteNumber << 2);

	dl_u8 count = playerData->lives;
	while (count--)
	{
		updateMetaSprite(tileIndex);
		oam_meta_spr(x, y + SCROLL_SPRITE_OFFSET, metasprite);  

		x += (PLAYERLIVES_ICON_SPACING << 1);
    }

	if (playerData->state == PLAYER_STATE_REGENERATION)
	{
		tileIndex = 0x6c + ((g_regenSpriteIndex + (REGEN_NUM_FRAMES * playerData->facingDirection)) << 2);

		updateMetaSprite(tileIndex);
		oam_meta_spr(x, y + SCROLL_SPRITE_OFFSET, metasprite);  
    }
}


__attribute__((section(".prg_rom_5")))
void updateControls(dl_u8 controllerIndex)
{
	dl_u8 padState = pad_poll(controllerIndex);

    // Check D-Pad
	dl_u8 leftDown = (padState & PAD_LEFT) != 0;
	dl_u8 rightDown = (padState & PAD_RIGHT) != 0;

	dl_u8 upDown = (padState & PAD_UP) != 0;
	dl_u8 downDown = (padState & PAD_DOWN) != 0;
			
	dl_u8 jumpDown = ((padState & PAD_A) != 0) || ((padState & PAD_B) != 0);
	dl_u8 startDown = (padState & PAD_START) != 0;

    joystickState_leftPressed = (!joystickState_leftDown) & leftDown;
    joystickState_rightPressed = (!joystickState_rightDown) & rightDown;
    joystickState_jumpPressed =  (!joystickState_jumpDown) & jumpDown;
	joystickState_startPressed = (!joystickState_startDown) & startDown;

    joystickState_leftDown = leftDown;
    joystickState_rightDown = rightDown;
    joystickState_upDown = upDown;
    joystickState_downDown = downDown;
    joystickState_jumpDown = jumpDown;
	joystickState_startDown = startDown;

#ifdef DEV_MODE
    dl_u8 debugStateDown = (padState & PAD_B) != 0;

    joystickState_debugStatePressed = (!joystickState_debugStateDown) & debugStateDown;
    joystickState_debugStateDown = debugStateDown;
#endif
}



//void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType);

const char palette[16] = 
{
    0x0f, 0x11, 0x27, 0x30 // black, gray, lt gray, white
};

int main(void)
{
	music_init(looping_effects_data);
	sounds_init(sound_effects_data);

	ppu_off(); // screen off
	vram_adr(NAMETABLE_A);


	// load the palettes
	pal_bg(palette);
	pal_spr(palette);
	oam_size(1);
	bank_spr(0);
	bank_bg(1);
	scroll(0, 240 - (SCROLL_SPRITE_OFFSET + 1));
	vram_fill(0, 32 * 28);
	ppu_on_all(); //	turn on screen

	set_vram_buffer();

	g_regenSpriteIndex = 0;

	// room draw setup


	//Game_ChangedRoomCallback = GameRunner_ChangedRoomCallback;

    Game_Init();

	dl_u8 controllerIndex = 0;

	for(;;) 
	{ 
		//set_music_speed(4);

        if (gameData_currentPlayerData != NULL)
        {
            controllerIndex = gameData_currentPlayerData->playerNumber;
        }

		set_prg_bank(5);
		updateControls(controllerIndex);
		set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);

		if (joystickState_startPressed)
		{
			gameData_paused = !gameData_paused;

			if (gameData_paused)
				Sound_StopAll();
		}

		// Game Loop

		if (!gameData_paused)
		{
			Game_Update();
		}

		ppu_wait_nmi(); // wait till beginning of the frame
		// the sprites are pushed from a buffer to the OAM during nmi

		// clear all sprites from sprite buffer
		oam_clear();

		set_prg_bank(5);
		m_drawRoomFunctions[gameData_currentRoom->roomNumber]();		
		set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);
	}

	return 0;
}


//void GameRunner_ChangedRoomCallback(const dl_u8 roomNumber, dl_s8 transitionType)
//{
	//UNUSED(roomNumber);

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
//}
__attribute__((noinline, section(".prg_rom_5")))
void drawPlayer()
{

	PlayerData* playerData = gameData_currentPlayerData;

	dl_u8 playerX = (playerData->x >> 8) << 1;
	dl_u8 playerY = (playerData->y >> 8);

	dl_u8 tileIndex;

	playerY += SCROLL_SPRITE_OFFSET;


	// draw player
	switch (playerData->state)
	{
	case PLAYER_STATE_SPLAT: 

		tileIndex = 0x8c + (playerData->splatFrameNumber * 6);

		updateMetaSprite(tileIndex);
		oam_meta_spr(playerX, playerY + 7, metasprite);  

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

		updateMetaSprite(tileIndex);
		oam_meta_spr(playerX, playerY, metasprite);  

		break;

	default: 
		g_playerTileIndex = 0x2 + (playerData->currentSpriteNumber << 2);

		updateMetaSprite(g_playerTileIndex);
		oam_meta_spr(playerX, playerY, metasprite);  
	}
}

dl_u8 tickTock;
__attribute__((noinline, section(".prg_rom_5")))
void drawChamber(void)
{
	drawPlayer();

	drawDoors();

	tickTock = !tickTock;

	PlayerData* playerData = gameData_currentPlayerData;
	dl_u16 currentTimer = playerData->roomTimers[playerData->currentRoom->roomNumber];

	if (tickTock)
	{
		Ball_Draw();
		Bird_Draw(currentTimer);
		drawDrops();
		drawUIPlayerLives(playerData);
		drawPickups();
	}
	else
	{
		drawPickups();
		drawDrops();
		Bird_Draw(currentTimer);
		Ball_Draw();
		drawUIPlayerLives(playerData);
	}

	drawTileText(gameData_string_timer, TIMER_DRAW_LOCATION);
	drawTileText(playerData->scoreString, SCORE_DRAW_LOCATION);
}
__attribute__((noinline, section(".prg_rom_5")))
void drawTitleScreen(void)
{
	drawDrops();
	dl_u8 x = gameData_numPlayers == 1 ? 32 : 128;
	oam_spr(x, 123 + SCROLL_SPRITE_OFFSET, 0x4a, 0);
}

__attribute__((noinline, section(".prg_rom_5")))
void drawTransition(void)
{
}

__attribute__((noinline, section(".prg_rom_5")))
void drawWipeTransition(void)
{
}

__attribute__((noinline, section(".prg_rom_5")))
void drawGetReadyScreen(void)
{
	drawDrops();
}

#define INITIAL_TRANSITION_DELAY 30

void transition_init(const Room* targetRoom)
{
	set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);
	const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[TITLESCREEN_ROOM_INDEX].backgroundDrawData;

	if (gameData_transitionRoomNumber == GET_READY_ROOM_INDEX)
		gameData_cleanBackground = (dl_u8*)getReadyScreen_cleanBackground;
	else 
		gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;

	ppu_off(); // screen off
	vram_adr(NTADR_A(0, 0));
	vram_write((dl_u8*)backgroundData->tileMap, 32 * 24);

	set_prg_bank(5);

	// init the clean background with the target room. 
	// it'll be revealed at the end of the transition.
	targetRoom->draw(gameData_transitionRoomNumber);

	ppu_on_all(); //	turn on screen

	set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);

	// setup screen transition
	gameData_transitionInitialDelay = INITIAL_TRANSITION_DELAY;
}

void transition_update(Room* room)
{
	UNUSED(room);

	// wait to draw anything until the delay is over
	if (gameData_transitionInitialDelay)
	{
		gameData_transitionInitialDelay--;
		return;
	}

	Game_EnterRoom(gameData_transitionRoomNumber);
}

void wipe_transition_init(const Room* targetRoom)
{
	UNUSED(targetRoom);

	set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);

	// setup screen transition
	gameData_transitionInitialDelay = INITIAL_TRANSITION_DELAY;
	gameData_transitionCurrentLine = 0;
	gameData_transitionFrameDelay = 0;

	const PlayerData* playerData = gameData_currentPlayerData;
	g_transitionDirection = playerData->lastDoor->xLocationInNextRoom < 50;
}

#define TILE_LINE_LENGTH 24
dl_u8 backgroundLine[TILE_LINE_LENGTH];
dl_u8 borderLine[TILE_LINE_LENGTH];

void wipe_transition_update(Room* room)
{
	UNUSED(room);

	// wait to draw anything until the delay is over
	if (gameData_transitionInitialDelay)
	{
		gameData_transitionInitialDelay--;
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

	const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[gameData_transitionRoomNumber].backgroundDrawData;
	const dl_u8* tilemapRunner = &backgroundData->tileMap[currentColumn];

	for (dl_u8 loop = 0; loop < 24; loop++)
	{
		backgroundLine[loop] = *tilemapRunner;
		borderLine[loop] = 18;
		tilemapRunner += 32;
	}

	dl_u16 addr = NTADR_A(currentColumn, 0);

	multi_vram_buffer_vert(backgroundLine, TILE_LINE_LENGTH, addr);

	if (currentColumn > 0 && currentColumn < 30)
	{
		addr += offset;
		multi_vram_buffer_vert(borderLine, TILE_LINE_LENGTH, addr);
	}

	gameData_transitionCurrentLine++;

	// we're done
	if (gameData_transitionCurrentLine == 32)
	{
		const BackgroundData* backgroundData = (const BackgroundData*)res_roomResources[gameData_transitionRoomNumber].backgroundDrawData;
		gameData_cleanBackground = (dl_u8*)backgroundData->cleanBackground;

		set_prg_bank(5);

		const Room* transitionRoom = g_rooms[gameData_transitionRoomNumber];
		transitionRoom->draw(gameData_transitionRoomNumber);

		set_prg_bank(roomToBankIndex[gameData_transitionRoomNumber]);

		Game_EnterRoom(gameData_transitionRoomNumber);
	}
}
