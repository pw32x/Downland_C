// mars headers
#include "mars.h"

// std headers
#include <stdio.h>
#include <string.h>

// project headers
#include "game_runner.h"
#include "downland_rom.h"

// game headers
#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"

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

	/* use letter-boxed 240p mode */
	if (Mars_IsPAL())
	{
		Mars_InitVideo(-240);
		Mars_SetMDColor(1, 0);
	}

	setup_palette();

    GameRunner_Init(&gameData, &resources);

	while (1)
	{
		GameRunner_Update(&gameData, &resources);
        GameRunner_Draw(&gameData, &resources);

		Mars_FlipFrameBuffers(1);
	}

	return 0;
}
