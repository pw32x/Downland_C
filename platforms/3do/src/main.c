// game headers
#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"

// 3do headers
#include "celutils.h"
#include "event.h"
#include "controlpad.h"

// project headers
#include "image_utils.h"
#include "display.h"
#include "game_runner.h"
#include "sound_manager.h"

#include <string.h>

GameData gameData;
Resources resources;

#define DOWNLAND_MEMORY_SIZE 18288
static dl_u8* g_memory = NULL;
static dl_u8* g_memoryEnd = NULL;

void* dl_alloc(dl_u32 size)
{
	dl_u8* newMemoryAlloc = g_memoryEnd;

	g_memoryEnd += size;

	return (void*)newMemoryAlloc;
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
    SoundManager_Play(soundIndex, loop);
}

void Sound_Stop(dl_u8 soundIndex)
{
    SoundManager_Stop(soundIndex);
}

#define CONTROL_FLAGS (ControlUp | ControlDown | ControlLeft | ControlRight | ControlStart | ControlA | ControlB | ControlC)
void Update_Controls(int controllerIndex, JoystickState* joystickState)
{
    uint32 button;
    bool leftDown;
    bool rightDown;
    bool upDown;
    bool downDown;
    bool jumpDown;
    bool startDown;
    bool debugStateDown;

    DoControlPad (controllerIndex + 1, &button, CONTROL_FLAGS);

    // Check D-Pad
    leftDown = (button & ControlLeft) != 0;
    rightDown = (button & ControlRight) != 0;
    upDown = (button & ControlUp) != 0;
    downDown = (button & ControlDown) != 0;
    jumpDown = (button & ControlA) || (button & ControlC);
    startDown = (button & ControlStart) != 0;

#ifdef DEV_MODE
    debugStateDown = button & ControlB;
#endif

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
    debugStateDown = false;//port->IsHeld(Digital::Button::B);

    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
    joystickState->debugStateDown = debugStateDown;
#endif
}

#define DOWNLAND_ROM_FILE_SIZE 8192
dl_u8* g_downlandRomFileBuffer = NULL;

const char* romFileNames[] = 
{
    "downland.rom",
    "downland.bin",
    "Downland V1.1 (1983) (26-3046) (Tandy) [a1].ccc"
};
#define ROM_FILENAMES_COUNT 3

static dl_u8 loadRom(const char* romPath, dl_u8** fileBuffer)
{
    int32 fileSize;

    *fileBuffer = (dl_u8*)LoadFile(romPath, &fileSize, MEMTYPE_ANY);

    if (*fileBuffer == NULL)
        return 0;

    if (fileSize != DOWNLAND_ROM_FILE_SIZE)
        return 0;

	return TRUE;
}

void int_to_bits(int n, char *out, int bits)
{
    int i;
    for (i = bits - 1; i >= 0; i--) 
    {
        out[bits - 1 - i] = (n & (1 << i)) ? '1' : '0';
    }
    out[bits] = '\0';
}

int main(int argc, char *argv[])
{
    const int clearColor = 0x00000000;

    bool romFoundAndLoaded = false;
    int loop;
    int controllerIndex = 0;

    g_memory = (dl_u8*)malloc(DOWNLAND_MEMORY_SIZE);
    g_memoryEnd = g_memory;

    for (loop = 0; loop < ROM_FILENAMES_COUNT; loop++)
    {
        if (loadRom(romFileNames[loop], &g_downlandRomFileBuffer) &&
            checksumCheckBigEndian(g_downlandRomFileBuffer, DOWNLAND_ROM_FILE_SIZE) &&
            ResourceLoaderBuffer_Init(g_downlandRomFileBuffer, DOWNLAND_ROM_FILE_SIZE, &resources))
        {
            romFoundAndLoaded = true;
            break;
        }

        break;
    }

    if (!romFoundAndLoaded)
        return -1;

    InitBasicDisplay();
    OpenMathFolio();
    OpenAudioFolio();
    InitControlPad(2);

    SoundManager_Init();

    clear(clearColor);
    swap();

    GameRunner_Init(&gameData, &resources);

    while(true)
    {
        if (gameData.currentPlayerData != NULL)
        {
            controllerIndex = gameData.currentPlayerData->playerNumber;
        }

        Update_Controls(controllerIndex, &gameData.joystickState);

        if (gameData.joystickState.startPressed)
        {
            gameData.paused = !gameData.paused;

            if (gameData.paused)
                SoundManager_PauseAll();
            else 
                SoundManager_ResumeAll();
        }

        if (!gameData.paused)
        {
            GameRunner_Update(&gameData, &resources);
        }

        SoundManager_Update();

        clear(clearColor);

        GameRunner_Draw(&gameData, &resources);
        
        display_and_swap();

        waitvbl();
    }

    return 0;
}

