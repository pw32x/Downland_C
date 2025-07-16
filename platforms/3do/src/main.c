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

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

#define CONTROL_FLAGS (ControlUp | ControlDown | ControlLeft | ControlRight | ControlStart | ControlA | ControlB | ControlC)
void Update_Controls(JoystickState* joystickState)
{
    uint32 button;
    bool leftDown;
    bool rightDown;
    bool upDown;
    bool downDown;
    bool jumpDown;
    bool startDown;
    bool debugStateDown;

    DoControlPad (1, &button, CONTROL_FLAGS);

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

int main(int argc, char* argv)
{
    const int clearColor = 0x00000000;

    bool romFoundAndLoaded = false;
    int loop;

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
    InitControlPad(2);

    clear(clearColor);
    swap();

    GameRunner_Init(&gameData, &resources);

    while(true)
    {
        Update_Controls(&gameData.joystickState);

        if (!gameData.paused)
        {
            GameRunner_Update(&gameData, &resources);
        }

        GameRunner_Draw(&gameData, &resources);

        //draw_printf(16,16,"x: %d",ConvertF16_32(x));
        //draw_printf(16,24,"y: %d",ConvertF16_32(y));
        //draw_printf(16,32,"x: %d",ConvertF16_32(zoom));
        //draw_printf(16,48,"y: %d",ConvertF16_32(angle));

        
        //int_to_bits(n, bitstr, 32);

        /*
        if (gameData.joystickState.leftPressed)
            draw_printf(16, 0,"left pressed");
        if (gameData.joystickState.rightPressed)
            draw_printf(16, 16,"right pressed");
        if (gameData.joystickState.upPressed)
            draw_printf(16, 32,"up pressed");
        if (gameData.joystickState.downPressed)
            draw_printf(16, 48,"down pressed");
        if (gameData.joystickState.jumpPressed)
            draw_printf(16, 64,"jump pressed");
        if (gameData.joystickState.startPressed)
            draw_printf(16, 80,"start pressed");

        draw_printf(16, 96, "button %d", button);

        if (button & ControlLeft)
            draw_printf(16, 0,"left pressed");
        */

        /*
        //CCB* ccb = &myCCB;
        pCCB = &myCCB;

        int_to_bits(pCCB->ccb_Flags, bitstr, 32); draw_printf(0, 0, "Flags: %s", bitstr);
        int_to_bits(pCCB->ccb_XPos, bitstr, 32); draw_printf(0, 16, "XPos: %s", bitstr);
        int_to_bits(pCCB->ccb_YPos, bitstr, 32); draw_printf(0, 32, "YPos: %s", bitstr);
        int_to_bits(pCCB->ccb_HDX, bitstr, 32); draw_printf(0, 48, "HDX: %s", bitstr);
        int_to_bits(pCCB->ccb_HDY, bitstr, 32); draw_printf(0, 64, "HDY: %s", bitstr);
        int_to_bits(pCCB->ccb_VDX, bitstr, 32); draw_printf(0, 80, "VDX: %s", bitstr);
        int_to_bits(pCCB->ccb_VDY, bitstr, 32); draw_printf(0, 96, "VDY: %s", bitstr);
        int_to_bits(pCCB->ccb_HDDX, bitstr, 32); draw_printf(0, 112, "HDDX: %s", bitstr);
        int_to_bits(pCCB->ccb_HDDY, bitstr, 32); draw_printf(0, 128, "HDDY: %s", bitstr);
        int_to_bits(pCCB->ccb_PIXC, bitstr, 32); draw_printf(0, 144, "PIXC: %s", bitstr);
        int_to_bits(pCCB->ccb_PRE0, bitstr, 32); draw_printf(0, 156, "PRE0: %s", bitstr);
        int_to_bits(pCCB->ccb_PRE1, bitstr, 32); draw_printf(0, 168, "PRE1: %s", bitstr);
        int_to_bits(pCCB->ccb_Width, bitstr, 32); draw_printf(0, 180, "Width: %s", bitstr);
        int_to_bits(pCCB->ccb_Height, bitstr, 32); draw_printf(0, 192, "Height: %s", bitstr);
        */

        //draw_printf(16, 16,"bytesRead %d",bytesRead);
        //draw_printf(16, 32,"load %d",loadResult);
        //draw_printf(16, 48,"little %d",littleResult);
        //draw_printf(16, 64,"big %d",bigResult);
        
        display_and_swap();

        waitvbl();
    }

    return 0;
}
