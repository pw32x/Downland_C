#include <gba_base.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_input.h>

#include <string.h>

#include "..\..\..\game\base_types.h"
#include "..\..\..\game\resource_types.h"
#include "..\..\..\game\resource_loader_buffer.h"
#include "..\..\..\game\checksum_utils.h"
#include "..\..\..\game\game.h"

#include "downland_rom.h"
#include "game_runner.h"

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

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

const u16 backgroundPalette[] = 
{
    RGB5(0,0,0),   // Transparent color (palette index 0)
    RGB5(0,0,31),  // Blue
    RGB5(31,20,0), // Orange
    RGB5(31,31,31),// white
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
	u16 keys = keysDown();

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

int main() 
{
	if (!checksumCheckLitteEndian(downland_rom, downland_rom_size))
		return -1;
	
	if (!ResourceLoaderBuffer_Init(downland_rom, downland_rom_size, &resources))
		return -1;

	memset(&gameData, 0, sizeof(GameData));

	GameRunner_Init(&gameData, &resources);

	// Set up the interrupt handlers
	irqInit();

	// Enable Vblank Interrupt to allow VblankIntrWait
	irqEnable(IRQ_VBLANK);

	// Allow Interrupts
	REG_IME = 1;

	// load palettes
	//

	volatile u16 *temppointer = BG_COLORS;
	for (int i = 0; i < 4; i++) 
	{
		*temppointer++ = backgroundPalette[i];
	}

    for (int i = 0; i < 4; i++) 
	{
        SPRITE_PALETTE[i] = spritePalette[i];
    }

	// set screen H and V scroll positions
	BG_OFFSET[0].x = 0; BG_OFFSET[0].y = 0;

	// set the screen base to 31 (0x600F800) and char base to 0 (0x6000000)
	BGCTRL[0] = SCREEN_BASE(31) |
				BG_16_COLOR |
			    BG_SIZE_0;

	// screen mode & background to display
	SetMode( MODE_0 | BG0_ON | MODE_0 | OBJ_ENABLE | OBJ_1D_MAP);

	// init the OAM 
	for (int i = 0; i < 128; i++) 
	{
		OAM[i].attr0 = ATTR0_DISABLED;
		OAM[i].attr1 = 0;
		OAM[i].attr2 = 0;
	}

	while (1) 
	{
		updateControls(&gameData.joystickState);

		GameRunner_Update(&gameData, &resources);

		VBlankIntrWait();

		GameRunner_Draw(&gameData, &resources);
	}
}
