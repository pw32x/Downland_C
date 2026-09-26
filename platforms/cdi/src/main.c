#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include <cdfm.h>
#include <memory.h>
#include <stdio.h>
#include <setsys.h>
#include "log.h"
#include "video.h"
#include "graphics.h"
#include "input.h"

#include "base_types.h"
#include "game.h"
#include "resource_types.h"

#include "game_runner.h"


GameData gameData;
Resources resources;

dl_u8 memory[18288];
dl_u8* memoryEnd = NULL;

void* dl_alloc(dl_u32 size)
{
	dl_u8* memoryAddr;
	if (memoryEnd == NULL)
	{
		memoryEnd = memory;
	}

	memoryAddr = memoryEnd;

	memoryEnd += size;

	return (void*)memoryAddr;
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

int intHandler(int sigCode)
{
	handleVideoSignal(sigCode);
	handleAudioSignal(sigCode);
}

void initSystem()
{
	intercept(intHandler);
	initVideo();
	initGraphics();
	initInput();
	initAudio();
}

void closeSystem()
{
	closeVideo();
	closeInput();
	closeAudio();
}


#define DOWNLAND_ROM_FILE_SIZE 8192
dl_u8 g_downlandRomFileBuffer[DOWNLAND_ROM_FILE_SIZE];

static dl_u8 loadRom(dl_u8* fileBuffer)
{
	int bytesRead = 0;
	int file = 0;

	file = open("DOWNLAND.ROM", READ_);
	if (file < 0)
		return 0;

	
	bytesRead = read(file, fileBuffer, DOWNLAND_ROM_FILE_SIZE);
	close(file);

	if (!bytesRead || bytesRead != DOWNLAND_ROM_FILE_SIZE)
		return 0;

	return 1;
}

dl_u16 updateControls(int controllerIndex, JoystickState* joystickState)
{
	dl_u16 buttonState = controllerIndex ? readInput2() : readInput1();

    // Check D-Pad
    dl_u8 leftDown = (buttonState & I_LEFT) != 0;
    dl_u8 rightDown = (buttonState & I_RIGHT) != 0;
    dl_u8 upDown = (buttonState & I_UP) != 0;
    dl_u8 downDown = (buttonState & I_DOWN) != 0;
    dl_u8 jumpDown = (buttonState & I_BUTTON_ANY);
    dl_u8 startDown = 0;//(buttonState & SEGA_CTRL_START) != 0;

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

void setPalette()
{
    u_int paletteData[6];

    paletteData[0] = cp_cbnk(0);                     /* select bank 0 (covers indices 0-63) */
    paletteData[1] = cp_clut(0, 0, 0, 0);            /* 0: black */
    paletteData[2] = cp_clut(1, 0, 64, 200);         /* 1: blue */
    paletteData[3] = cp_clut(2, 255, 128, 0);        /* 2: orange */
    paletteData[4] = cp_clut(3, 255, 255, 255);      /* 3: white */
	paletteData[5] = cp_clut(4, 0, 0, 0);			 /* 4: black */

    dc_wrfct(videoPath, fctA, FCT_PAL_START, 6, paletteData);
	dc_wrfct(videoPath, fctB, FCT_PAL_START, 6, paletteData);
}

int main(int argc, 	char* argv[])
{
    dl_u8 romFoundAndLoaded = 0;
	dl_u8 controllerIndex = 0;
	dl_u16 buttonState;

    if (loadRom(g_downlandRomFileBuffer) &&
        checksumCheckBigEndian(g_downlandRomFileBuffer, DOWNLAND_ROM_FILE_SIZE) &&
        ResourceLoaderBuffer_Init(g_downlandRomFileBuffer, DOWNLAND_ROM_FILE_SIZE, &resources))
    {
        romFoundAndLoaded = 1;
    }

    if (!romFoundAndLoaded)
        return exit(-1);

	initSystem();
	
	memset(&gameData, 0, sizeof(GameData));

	setPalette();

	GameRunner_Init(&gameData, &resources);

	dc_ssig(videoPath, SIG_BLANK, 0);
	
	while (1) 
	{
		if (!frameDone) 
		{
			continue; /* Wait for SIG_BLANK */
		}

		frameDone = 0;

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

		swapVideoBuffers();

		// setup vblank signal
		dc_ssig(videoPath, SIG_BLANK, 0);
	}

	closeSystem();
	exit(0);
}
