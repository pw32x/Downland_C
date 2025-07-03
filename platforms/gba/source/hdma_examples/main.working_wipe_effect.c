#include <gba_base.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_input.h>
#include <gba_dma.h>

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

//GameData gameData;
//Resources resources;
//
//static dl_u8 memory[18288];
//static dl_u8* memoryEnd = NULL;
//
//void* dl_alloc(dl_u32 size)
//{
//	if (memoryEnd == NULL)
//	{
//		memoryEnd = memory;
//	}
//
//	dl_u8* memory = memoryEnd;
//
//	memoryEnd += size;
//
//	return (void*)memory;
//}
//
//mm_sound_effect sounds[SOUND_NUM_SOUNDS] =
//{
//	// id, rate, handle, volume, panning
//	{ { SFX_JUMP }, (int)(1.0f * (1<<10)), 0, 255, 128},
//	{ { SFX_LAND }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//	{ { SFX_TRANSITION }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//	{ { SFX_SPLAT }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//	{ { SFX_PICKUP }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//	{ { SFX_RUN }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//	{ { SFX_CLIMB_UP }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//	{ { SFX_CLIMB_DOWN }, (int)(1.0f * (1<<10)), 0, 255, 128 },
//};
//
//#define NO_SOUND 0xffff
//
//mm_sfxhand soundHandles[SOUND_NUM_SOUNDS] = 
//{ 
//	NO_SOUND, 
//	NO_SOUND,
//	NO_SOUND,
//	NO_SOUND,
//	NO_SOUND,
//	NO_SOUND,
//	NO_SOUND,
//	NO_SOUND
//};
//
//void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
//{
//	if (loop)
//	{
//		if (soundHandles[soundIndex] != NO_SOUND)
//			return;
//		soundHandles[soundIndex] = mmEffectEx(&sounds[soundIndex]);
//	}
//	else
//	{
//		soundHandles[soundIndex] = mmEffectEx(&sounds[soundIndex]);
//	}
//}
//
//void Sound_Stop(dl_u8 soundIndex)
//{
//	mmEffectCancel(soundHandles[soundIndex]);
//	soundHandles[soundIndex] = NO_SOUND;
//}
//
//const u16 backgroundPalette[] = 
//{
//    RGB5(0,0,0),   // Transparent color (palette index 0)
//    RGB5(0,0,31),  // Blue
//    RGB5(31,20,0), // Orange
//    RGB5(31,31,31),// white
//	RGB5(0,0,0),   // Non transparent black
//};
//
//const unsigned short spritePalette[4] = 
//{
//    RGB5(0,0,0),   // Transparent color (palette index 0)
//    RGB5(0,0,31),  // Blue
//    RGB5(31,20,0), // Orange
//    RGB5(31,31,31),// white
//};
//
//void updateControls(JoystickState* joystickState)
//{
//	scanKeys();
//	u16 keys = keysHeld();
//
//    // Check D-Pad
//    bool leftDown = keys & KEY_LEFT;
//    bool rightDown = keys & KEY_RIGHT;
//    bool upDown = keys & KEY_UP;
//    bool downDown = keys & KEY_DOWN;
//    bool jumpDown = keys & KEY_A;
//    bool startDown = keys & KEY_START;
//
//    joystickState->leftPressed = !joystickState->leftDown & leftDown;
//    joystickState->rightPressed = !joystickState->rightDown & rightDown;
//    joystickState->upPressed = !joystickState->upDown & upDown;
//    joystickState->downPressed =  !joystickState->downDown & downDown;
//    joystickState->jumpPressed =  !joystickState->jumpDown & jumpDown;
//    joystickState->startPressed = !joystickState->startDown & startDown;
//
//    joystickState->leftReleased = joystickState->leftDown & !leftDown;
//    joystickState->rightReleased = joystickState->rightDown & !rightDown;
//    joystickState->upReleased = joystickState->upDown & !upDown;
//    joystickState->downReleased =  joystickState->downDown & !downDown;
//    joystickState->jumpReleased =  joystickState->jumpDown & !jumpDown;
//    joystickState->startReleased = joystickState->startPressed & !startDown;
//
//    joystickState->leftDown = leftDown;
//    joystickState->rightDown = rightDown;
//    joystickState->upDown = upDown;
//    joystickState->downDown = downDown;
//    joystickState->jumpDown = jumpDown;
//    joystickState->startDown = startDown;
//
//#ifdef DEV_MODE
//    bool debugStateDown = keys & KEY_B;
//
//    joystickState->debugStatePressed = !joystickState->debugStateDown & debugStateDown;
//    joystickState->debugStateReleased = joystickState->debugStatePressed & !debugStateDown;
//    joystickState->debugStateDown = debugStateDown;
//#endif
//}
//
//u16 hdma_table[160];
//int line_cutoff = 0;
//
//void prepare_hdma_table(int cutoff) 
//{
//    for (int i = 0; i < 160; i++) 
//	{
//        hdma_table[i] = 159 - i;//(i < cutoff) ? 0 : 8;
//    }
//}
//
//void setup_hdma() 
//{
//	//u16 table[160]; for (int i=0;i<160;i++) table[i] = i;
//	//REG_DMA3SAD = (u32)table;
//	//REG_DMA3DAD = (u32)&REG_BG1VOFS;
//	//REG_DMA3CNT = DMA_ENABLE|DMA_SPECIAL|DMA_DST_FIXED|DMA_REPEAT|DMA16|DMA_HBLANK|160;
//
//    REG_DMA3SAD = (u32)hdma_table;
//    REG_DMA3DAD = (u32)&REG_BG1VOFS;//(u32)&REG_BG2Y;
//    REG_DMA3CNT = DMA_ENABLE | 
//			      DMA_SPECIAL | 
//				  DMA_DST_FIXED | 
//				  DMA_REPEAT | 
//				  DMA16 | 
//				  DMA_HBLANK | 
//				  160;
//}
//
//int main() 
//{
//	if (!checksumCheckLitteEndian(downland_rom, downland_rom_size))
//		return -1;
//	
//	if (!ResourceLoaderBuffer_Init(downland_rom, downland_rom_size, &resources))
//		return -1;
//
//	memset(&gameData, 0, sizeof(GameData));
//
//	GameRunner_Init(&gameData, &resources);
//
//	// Set up the interrupt handlers
//	irqInit();
//
//	// Maxmod requires the vblank interrupt to reset sound DMA.
//	// Link the VBlank interrupt to mmVBlank, and enable it. 
//	irqSet( IRQ_VBLANK, mmVBlank );
//	irqEnable(IRQ_VBLANK | IRQ_HBLANK);
//
//	REG_DISPSTAT |= (1<<4); //DISPSTAT_HBLANK_IRQ;
//
//	// initialise maxmod with soundbank and 8 channels
//    mmInitDefault( (mm_addr)soundbank_bin, 8 );
//
//	// Allow Interrupts
//	REG_IME = 1;
//
//	// load palettes
//	//
//
//	volatile u16 *temppointer = BG_COLORS;
//	for (int i = 0; i < 5; i++) 
//	{
//		*temppointer++ = backgroundPalette[i];
//	}
//
//    for (int i = 0; i < 4; i++) 
//	{
//        SPRITE_PALETTE[i] = spritePalette[i];
//    }
//
//	// set screen H and V scroll positions
//	BG_OFFSET[UI_BACKGROUND_INDEX].x = 0; 
//	BG_OFFSET[UI_BACKGROUND_INDEX].y = 0;
//	BG_OFFSET[GAME_BACKGROUND_INDEX].x = 7; 
//	BG_OFFSET[GAME_BACKGROUND_INDEX].y = 13;
//
//	// set the screen base to 31 (0x600F800) and char base to 0 (0x6000000)
//	BGCTRL[UI_BACKGROUND_INDEX] = SCREEN_BASE(UI_TILEMAP_INDEX) |
//								  BG_16_COLOR |
//								  BG_PRIORITY(UI_BACKGROUND_PRIORITY) |
//								  BG_SIZE_0;
//
//	BGCTRL[TRANSITION_BACKGROUND_INDEX] = SCREEN_BASE(TRANSITION_TILEMAP_INDEX) |
//									      BG_256_COLOR |
//									      BG_PRIORITY(TRANSITION_BACKGROUND_PRIORITY) |
//									      BG_SIZE_1;
//
//	// init the OAM 
//	for (int i = 0; i < 128; i++) 
//	{
//		OAM[i].attr0 = ATTR0_DISABLED;
//		OAM[i].attr1 = 0;
//		OAM[i].attr2 = 0;
//	}
//
//    // Affine matrix (identity, no scale/rotation)
//    REG_BG2PA = 256;
//    REG_BG2PB = 0;
//    REG_BG2PC = 0;
//    REG_BG2PD = 256;
//
//    REG_BG2X = 0;
//    REG_BG2Y = 0;
//
//    prepare_hdma_table(line_cutoff);
//    setup_hdma();
//
//	while (1) 
//	{
//		updateControls(&gameData.joystickState);
//
//		GameRunner_Update(&gameData, &resources);
//
//		VBlankIntrWait();
//		mmFrame();
//
//        // Update line wipe every frame
//        if (line_cutoff < 160) 
//		{
//            line_cutoff++;
//        }
//		else
//		{
//			line_cutoff = 0;
//		}
//
//        prepare_hdma_table(line_cutoff);
//
//		GameRunner_Draw(&gameData, &resources);
//	}
//}
//

dl_u8 counter;
s32 hdma_table[160];

void update_hdma_table() 
{
    dl_u8 counterToUse = counter >> 1;
    if (counterToUse < 32)
    {
        for (int i = 0; i < 160; i++) 
        {
            dl_u8 index = (i + 1) % 160;
            dl_u8 currentLine = index % 32;

            dl_u16 value = 2; // default blue line

            if (currentLine > counterToUse)
            {
                value = 1; // show non-transparent black line
            }
            else if (currentLine < counterToUse)
            {
                value = 0; // transparent black line
            }

            hdma_table[i] = value << 8; 
        }
    }

    counter++;
    if (counter > 250)
        counter = 0;
}

void setup_hdma() 
{
    REG_DMA3CNT = 0;

    REG_DMA3SAD = (u32)hdma_table;
    REG_DMA3DAD = (u32)&REG_BG2Y;
    REG_DMA3CNT = DMA_ENABLE | 
        DMA_HBLANK | 
        DMA_REPEAT | 
        DMA_DST_FIXED |
        DMA_SRC_INC | 
        DMA32 | 
        DMA_HBLANK |
        1;
}

void vblank_handler() 
{
    // on Vblank, update the table and restart the HDMA
    //

    // Update the HDMA table with new sine wave values
    update_hdma_table();
    
    // Set up HDMA
    setup_hdma();
}

int main() 
{
    // Init GBA
    irqInit();
    irqEnable(IRQ_VBLANK);
    irqSet(IRQ_VBLANK, vblank_handler);

    // Set mode 0 and enable BG1
    REG_DISPCNT = MODE_1 | BG1_ENABLE | BG2_ENABLE | OBJ_1D_MAP;

    REG_BG1CNT = BG_SIZE_0 | BG_16_COLOR | SCREEN_BASE(31) | CHAR_BASE(0) | BG_PRIORITY(1);
    REG_BG2CNT = BG_SIZE_1 | BG_256_COLOR | SCREEN_BASE(30) | CHAR_BASE(1) | BG_PRIORITY(0);


    // Affine matrix (identity, no scale/rotation)
    REG_BG2PA = 256;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 256;

    REG_BG2X = 0;
    REG_BG2Y = 0;

    // setup bg1
    u16* charBase = (u16*)CHAR_BASE_BLOCK(0);
    for (int i = 0; i < 8 * (8 / 4); i++) 
    {
        charBase[i] = 0x0000;       // transparent tile
        charBase[i + 16] = 0x1111;  // blue tile
        charBase[i + 32] = 0x2222;  // orange tile
        charBase[i + 48] = 0x3333;  // white tile
    }

    u16* bgMap = (u16*)SCREEN_BASE_BLOCK(31);
    for (int i = 0; i < 32 * 32; i++) 
    {
        dl_u8 value = (i % 2) + 1;
        bgMap[i] = value;
    }

    // Palette
    BG_COLORS[0] = RGB5(0, 0, 0);    // Black (transparent)
    BG_COLORS[1] = RGB5(0, 0, 31);   // Blue
    BG_COLORS[2] = RGB5(31, 20,0);   // orange
    BG_COLORS[3] = RGB5(31, 31, 31); // White
    BG_COLORS[4] = RGB5(0, 0, 0);    // non-transparent black

    // setup bg2
    charBase = (u16*)CHAR_BASE_BLOCK(1);

    for (int i = 0; i < 4; i++)
    {
        charBase[i] = 0x0000;
        charBase[i + 4] = 0x0404; // non transparent black
        charBase[i + 8] = 0x0101; // blue
    }

    bgMap = (u16*)SCREEN_BASE_BLOCK(30);
    // fill the first line of bg2 with empty tile. 
    // we don't need more.
    for (int i = 0; i < 16 ; i++) 
    {
        bgMap[i] = 0;
    }

    update_hdma_table();
    setup_hdma();

    while (1) 
    {
        VBlankIntrWait();
        // Nothing needed here; HDMA runs in hardware during HBlank
    }

    return 0;
}
