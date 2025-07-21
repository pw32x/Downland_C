/* marsonly.c */

#include "mars.h"
#include "downland_rom.h"

// game headers
#include "game_types.h"
#include "resource_types.h"
#include "checksum_utils.h"
#include "resource_loader_buffer.h"


void setup_gray_palette()
{
	unsigned short* cram = (unsigned short *)&MARS_CRAM;

	if ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0)
		return;

	for (int i = 0; i < 256; i++) 
	{
		unsigned short b1 = (i & 0x1f) << 10;
		unsigned short g1 = (i & 0x1f) << 5;
		unsigned short r1 = (i & 0x1f) << 0;

		cram[i] = r1 | g1 | b1;
	}

	// with MARS_VDP_PRIO_32X active, setup color 0's highest bit to tell the 32X 
	// hardware to draw the Genesis graphics underneath.
	cram[0] = (1 << 15);
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

void draw_square(int x, int y, int size)
{
	volatile unsigned char* framebuffer = (unsigned char*)(&MARS_FRAMEBUFFER + 0x100);

	for (int loopy = 0; loopy < size; loopy++)
	{
		for (int loopx = 0; loopx < size; loopx++)
		{
			framebuffer[(x + loopx) + ((y + loopy) * 320)] = 15;
		}
	}
}

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224

int main(void)
{
	/* clear screen */
	Mars_Init();

	Mars_CommSlaveClearCache();

	/* use letter-boxed 240p mode */
	if (Mars_IsPAL())
	{
		Mars_InitVideo(-240);
		Mars_SetMDColor(1, 0);
	}

	setup_gray_palette();


	const unsigned char* downlandRom = _binary_downland_rom_start;
	const unsigned char* downlandRomEnd = _binary_downland_rom_end;
	const unsigned int  downlandRomSize = _binary_downland_rom_size;


	int x = 40;
	int y = 50;
	int directionx = 1;
	int directiony = 1;

	int square_size = 100;

	int screen_limitx = SCREEN_WIDTH - directionx - square_size;
	int screen_limity = SCREEN_HEIGHT - directiony - square_size;

	Mars_SetMDCrsr(2, 20);
	Mars_MDPutString("Hello world from 32x Side");

	while (1)
	{
		clear_framebuffer();



		x += directionx;
		y += directiony;

		if (x < 0)
		{
			x = 0;
			directionx = -directionx;
		}

		if (x  > screen_limitx - 1)
		{
			x  = screen_limitx - 1;
			directionx = -directionx;
		}

		if (y < 0)
		{
			y = 0;
			directiony = -directiony;
		}

		if (y  > screen_limity - 1)
		{
			y = screen_limity - 1;
			directiony = -directiony;
		}

        draw_square(x, y, square_size);

		Mars_FlipFrameBuffers(1);
	}

	return 0;
}
