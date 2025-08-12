#include <gba_base.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_input.h>

#include <string.h>

#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"
#include "..\..\..\game\dl_sound.h"

#include "..\..\..\game\base_types.h"
#include "..\..\..\game\resource_types.h"
#include "..\..\..\game\resource_loader_buffer.h"
#include "..\..\..\game\checksum_utils.h"
#include "..\..\..\game\game.h"

#include "downland_rom.h"
#include "game_runner.h"
#include "gba_defines.h"
#include "transition_effect.h"

GameData gameData;
Resources resources;

static dl_u8 memory[18288];
static dl_u8* memoryEnd = NULL;

void* dl_alloc(dl_u32 size)
{
	if (memoryEnd == NULL)
	{
		memoryEnd = memory;
	}

	dl_u8* memory = memoryEnd;

	memoryEnd += size;

	return (void*)memory;
}

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{
    memset(source, value, count);
}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
    memcpy(destination, source, count);
}

mm_sound_effect sounds[SOUND_NUM_SOUNDS] =
{
	// id, rate, handle, volume, panning
	{ { SFX_JUMP }, (int)(1.0f * (1<<10)), 0, 255, 128},
	{ { SFX_LAND }, (int)(1.0f * (1<<10)), 0, 255, 128 },
	{ { SFX_TRANSITION }, (int)(1.0f * (1<<10)), 0, 255, 128 },
	{ { SFX_SPLAT }, (int)(1.0f * (1<<10)), 0, 255, 128 },
	{ { SFX_PICKUP }, (int)(1.0f * (1<<10)), 0, 255, 128 },
	{ { SFX_RUN }, (int)(1.0f * (1<<10)), 0, 255, 128 },
	{ { SFX_CLIMB_UP }, (int)(1.0f * (1<<10)), 0, 255, 128 },
	{ { SFX_CLIMB_DOWN }, (int)(1.0f * (1<<10)), 0, 255, 128 },
};

#define NO_SOUND 0xffff

mm_sfxhand soundHandles[SOUND_NUM_SOUNDS] = 
{ 
	NO_SOUND, 
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND
};

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
	if (loop)
	{
		if (soundHandles[soundIndex] != NO_SOUND)
			return;
		soundHandles[soundIndex] = mmEffectEx(&sounds[soundIndex]);
	}
	else
	{
		soundHandles[soundIndex] = mmEffectEx(&sounds[soundIndex]);
	}
}

void Sound_Stop(dl_u8 soundIndex)
{
	mmEffectCancel(soundHandles[soundIndex]);
	soundHandles[soundIndex] = NO_SOUND;
}

void Sound_StopAll()
{
	for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
		Sound_Stop(loop);
}

const u16 backgroundPalette[] = 
{
    RGB5(0,0,0),   // Transparent color (palette index 0)
    RGB5(0,0,31),  // Blue
    RGB5(31,20,0), // Orange
    RGB5(31,31,31),// white
	RGB5(0,0,0),   // Non transparent black
};

const unsigned short spritePalette[4] = 
{
    RGB5(0,0,0),   // Transparent color (palette index 0)
    RGB5(0,0,31),  // Blue
    RGB5(31,20,0), // Orange
    RGB5(31,31,31),// white
};

void updateControls(JoystickState* joystickState)
{
	scanKeys();
	u16 keys = keysHeld();

    // Check D-Pad
    bool leftDown = keys & KEY_LEFT;
    bool rightDown = keys & KEY_RIGHT;
    bool upDown = keys & KEY_UP;
    bool downDown = keys & KEY_DOWN;
    bool jumpDown = keys & KEY_A;
    bool startDown = keys & KEY_START;

    joystickState->leftPressed = !joystickState->leftDown & leftDown;
    joystickState->rightPressed = !joystickState->rightDown & rightDown;
    joystickState->upPressed = !joystickState->upDown & upDown;
    joystickState->downPressed =  !joystickState->downDown & downDown;
    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;
    joystickState->startPressed = !joystickState->startDown & startDown;

    joystickState->leftReleased = joystickState->leftDown & !leftDown;
    joystickState->rightReleased = joystickState->rightDown & !rightDown;
    joystickState->upReleased = joystickState->upDown & !upDown;
    joystickState->downReleased =  joystickState->downDown & !downDown;
    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;
    joystickState->startReleased = joystickState->startPressed & !startDown;

    joystickState->leftDown = leftDown;
    joystickState->rightDown = rightDown;
    joystickState->upDown = upDown;
    joystickState->downDown = downDown;
    joystickState->jumpDown = jumpDown;
    joystickState->startDown = startDown;

#ifdef DEV_MODE
    bool debugStateDown = keys & KEY_B;

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}


void vblank_handler() 
{
    mmVBlank();

    // Update the HDMA effect
    updateTransitionHDMATable();
    setupTransitionHDMA();
}

int main() 
{
	g_transitionCounter = TRANSITION_TURN_ON;

	if (!checksumCheckLitteEndian(downland_rom, downland_rom_size))
		return -1;
	
	if (!ResourceLoaderBuffer_Init(downland_rom, downland_rom_size, &resources))
		return -1;

	memset(&gameData, 0, sizeof(GameData));

	GameRunner_Init(&gameData, &resources);

	// Set up the interrupt handlers
	irqInit();

	// Maxmod requires the vblank interrupt to reset sound DMA.
	// Link the VBlank interrupt to mmVBlank, and enable it. 
	irqSet(IRQ_VBLANK, vblank_handler);
	irqEnable(IRQ_VBLANK);

	// initialise maxmod with soundbank and 8 channels
    mmInitDefault( (mm_addr)soundbank_bin, 8 );

	// Allow Interrupts
	REG_IME = 1;

	// load palettes
	//

	volatile u16 *temppointer = BG_COLORS;
	for (int i = 0; i < 5; i++) 
	{
		*temppointer++ = backgroundPalette[i];
	}

    for (int i = 0; i < 4; i++) 
	{
        SPRITE_PALETTE[i] = spritePalette[i];
    }

	// set screen H and V scroll positions
	BG_OFFSET[UI_BACKGROUND_INDEX].x = 0; 
	BG_OFFSET[UI_BACKGROUND_INDEX].y = 0;
	BG_OFFSET[GAME_BACKGROUND_INDEX].x = 7; 
	BG_OFFSET[GAME_BACKGROUND_INDEX].y = 13;

	// set the screen base to 31 (0x600F800) and char base to 0 (0x6000000)
	BGCTRL[UI_BACKGROUND_INDEX] = SCREEN_BASE(UI_TILEMAP_INDEX) |
								  BG_16_COLOR |
								  BG_PRIORITY(UI_BACKGROUND_PRIORITY) |
								  BG_SIZE_0;

	BGCTRL[TRANSITION_BACKGROUND_INDEX] = SCREEN_BASE(TRANSITION_TILEMAP_INDEX) |
									      BG_256_COLOR |
									      BG_PRIORITY(TRANSITION_BACKGROUND_PRIORITY) |
									      BG_SIZE_1;

	// init the OAM 
	for (int i = 0; i < 128; i++) 
	{
		OAM[i].attr0 = ATTR0_DISABLED;
		OAM[i].attr1 = 0;
		OAM[i].attr2 = 0;
	}

    // Affine matrix (identity, no scale/rotation)
    REG_BG2PA = 256;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 256;

    REG_BG2X = 0;
    REG_BG2Y = 0;

	// setp HDMA effect
    updateTransitionHDMATable();
    setupTransitionHDMA();

	while (1) 
	{
		updateControls(&gameData.joystickState);

		if (gameData.joystickState.startPressed)
		{
			gameData.paused = !gameData.paused;

			if (gameData.paused)
				Sound_StopAll();
		}

		if (!gameData.paused)
		{
			GameRunner_Update(&gameData, &resources);
		}

		VBlankIntrWait();
		mmFrame();

		GameRunner_Draw(&gameData, &resources);
	}
}
