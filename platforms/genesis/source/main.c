#include <genesis.h>
#include "resources.h"

// sgdk headers
#include <memory.h>

// project headers
#include "game_runner.h"



// game headers
#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"
#include "dl_sound.h"
#include "dl_platform.h"

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

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

void Sound_StopAll()
{
}


void updateControls(JoystickState* joystickState)
{
	dl_u16 state = JOY_readJoypad(0);

    // Check D-Pad
    bool leftDown = (state & BUTTON_LEFT) != 0;
    bool rightDown = (state & BUTTON_RIGHT) != 0;
    bool upDown = (state & BUTTON_UP) != 0;
    bool downDown = (state & BUTTON_DOWN) != 0;
    bool jumpDown = (state & BUTTON_A) != 0;
    bool startDown = (state & BUTTON_START) != 0;

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
    bool debugStateDown = keys & BUTTON_B;

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}

dl_u16 palette[] = 
{
	RGB8_8_8_TO_VDPCOLOR(0,		0,   0),
	RGB8_8_8_TO_VDPCOLOR(0,		0, 255),
	RGB8_8_8_TO_VDPCOLOR(255, 165,   0),
	RGB8_8_8_TO_VDPCOLOR(255, 255, 255),
	RGB8_8_8_TO_VDPCOLOR(0,		0,   0),
};

// Entry Point
int main(bool hardReset)
{
    if (!hardReset)
	{
        SYS_hardReset();
	}

	int result = 0;
	(void)result;
	dl_u16 romSize = sizeof(downland_rom);

	if (romSize != DOWNLAND_ROM_SIZE)
	{
		result = -3;
	}
	else if (!checksumCheckBigEndian(downland_rom, 
								     romSize))
	{
		result = -1;
	}
	else if (!ResourceLoaderBuffer_Init(downland_rom, 
									    romSize, 
									    &resources))
	{
		result = -2;
	}

    // Initialize systems
    XGM2_loadDriver(TRUE);
    JOY_init();

    // Load sprite palettes and set the text palette
	PAL_setColors(PAL0, palette, 5, DMA);
    PAL_setPalette(PAL1, player_sprite.palette->data, DMA);
    PAL_setPalette(PAL2, explosion_sprite.palette->data, DMA);
    PAL_setPalette(PAL3, enemy_sprite.palette->data, DMA);
    VDP_setTextPalette(PAL1);


	/*
	switch (result)
	{
	case -3: VDP_drawText("Rom not the correct size", 12, 12); break;
	case -2: VDP_drawText("Resource loader failed", 12, 13); break;
	case -1: VDP_drawText("Checksum failed", 12, 14); break;
	default: VDP_drawText("Load resources successful", 12, 15); break;
	}
	*/

	memset(&gameData, 0, sizeof(GameData));

	GameRunner_Init(&gameData, &resources);

    while (TRUE)
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



        SYS_doVBlankProcess();
		GameRunner_Draw(&gameData, &resources);
    }

    return 0;
}

