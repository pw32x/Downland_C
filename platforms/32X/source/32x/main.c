// mars headers
#include "mars.h"

// std headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// project headers
#include "game_runner.h"
#include "downland_rom.h"
#include "sound_manager.h"
#include "files.h"

// game headers
#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"
#include "dl_sound.h"

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


SoundHandle soundHandles[SOUND_NUM_SOUNDS];

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
    const SoundFile* soundFile = &soundFiles[soundIndex];

	if (loop && SoundManager_isSoundEffectPlaying(soundHandles[soundIndex]))
		return;

    soundHandles[soundIndex] = SoundManager_playSoundEffect(soundFile->filePtr, 
                                                            soundFile->fileSize, 
                                                            soundFile->sampleRate, 
                                                            loop, 
                                                            64, 
                                                            128); 
}

void Sound_Stop(dl_u8 soundIndex)
{
	SoundManager_stopSoundEffect(soundHandles[soundIndex]);
	soundHandles[soundIndex] = INVALID_SOUND_HANDLE;
}

void Sound_StopAll()
{
	for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
		Sound_Stop(loop);
}

#define RGB5(r,g,b)	((r)|((g)<<5)|((b)<<10))
void setup_palette()
{
	unsigned short* cram = (unsigned short *)&MARS_CRAM;

	if ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0)
		return;

    cram[0] = RGB5(0,0,0);   // Transparent color (palette index 0)
    cram[1] = RGB5(0,0,31);  // Blue
    cram[2] = RGB5(31,20,0); // Orange
    cram[3] = RGB5(31,31,31);// white
	cram[4] = RGB5(0,0,0);   // Non transparent black
}

void clear_framebuffer()
{
	volatile int* p, * p_end;

	p = (int*)(&MARS_FRAMEBUFFER + 0x100);
	p_end = (int*)p + 320 / 4 * mars_framebuffer_height;
	do {
		*p = 0;
	} while (++p < p_end);
}


dl_u16 updateControls(int controllerIndex, JoystickState* joystickState)
{
	//dl_u16 buttonState = Mars_ReadController(controllerIndex); // this doesn't work. Don't know why.

	dl_u16 buttonState = controllerIndex ? MARS_SYS_COMM10 : MARS_SYS_COMM8; // get the port directly

    // Check D-Pad
    dl_u8 leftDown = (buttonState & SEGA_CTRL_LEFT) != 0;
    dl_u8 rightDown = (buttonState & SEGA_CTRL_RIGHT) != 0;
    dl_u8 upDown = (buttonState & SEGA_CTRL_UP) != 0;
    dl_u8 downDown = (buttonState & SEGA_CTRL_DOWN) != 0;
    dl_u8 jumpDown = (buttonState & SEGA_CTRL_A) || (buttonState & SEGA_CTRL_C);
    dl_u8 startDown = (buttonState & SEGA_CTRL_START) != 0;

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
    bool debugStateDown = (buttonState & SEGA_CTRL_B);

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
	
	return buttonState;
}

int main(void)
{
	int result = 0;

	int romSize = binary_gamedata_downland_rom_end - binary_gamedata_downland_rom_start;

	if (romSize != DOWNLAND_ROM_SIZE)
	{
		result = -3;
	}
	else if (!checksumCheckBigEndian(binary_gamedata_downland_rom_start, 
								  romSize))
	{
		result = -1;
	}
	else if (!ResourceLoaderBuffer_Init(binary_gamedata_downland_rom_start, 
								   romSize, 
								   &resources))
	{
		result = -2;
	}

	memset(&gameData, 0, sizeof(GameData));

	Mars_Init();
	Mars_CommSlaveClearCache();
	Mars_DetectInputDevices();

	/* use letter-boxed 240p mode */
	if (Mars_IsPAL())
	{
		Mars_InitVideo(-240);
		Mars_SetMDColor(1, 0);
	}

	setup_palette();

    GameRunner_Init(&gameData, &resources);

	memset(soundHandles, INVALID_SOUND_HANDLE, sizeof(soundHandles));
	SoundManager_waitUntilInitialized();

	int controllerIndex = 0;

	dl_u16 buttonState = 0;

	while (1)
	{
        if (gameData.currentPlayerData != NULL)
        {
            controllerIndex = gameData.currentPlayerData->playerNumber;
        }

		buttonState = updateControls(controllerIndex, &gameData.joystickState);

		if (gameData.joystickState.startPressed)
		{
			gameData.paused = !gameData.paused;

		}

		if (!gameData.paused)
		{
			GameRunner_Update(&gameData, &resources);
		}

        GameRunner_Draw(&gameData, &resources);

		// draw the joystick state on screen
		/*
		volatile unsigned char* frameBuffer = (unsigned char*)(&MARS_FRAMEBUFFER + 0x100);
		for (int loop = 0; loop < 16; loop++)
		{
			frameBuffer[(loop * 4) + 32] = buttonState & 1 ? 1 : 2;
			buttonState >>= 1;
		}
		*/

		Mars_FlipFrameBuffers(1);
	}

	SoundManager_shutdown();

	return 0;
}
