#include "draw_value.h"
#include "../cdi_hw/graphics.h"

// small blocky 3x5 retro digit font
static const dl_u8 g_digitsFont[10][5] = 
{
	{ 7, 5, 5, 5, 7 }, // 0
	{ 2, 6, 2, 2, 7 }, // 1
	{ 7, 1, 7, 4, 7 }, // 2
	{ 7, 1, 7, 1, 7 }, // 3
	{ 5, 5, 7, 1, 1 }, // 4
	{ 7, 4, 7, 1, 7 }, // 5
	{ 7, 4, 7, 5, 7 }, // 6
	{ 7, 1, 1, 1, 1 }, // 7
	{ 7, 5, 7, 5, 7 }, // 8
	{ 7, 5, 7, 1, 7 }  // 9
};

#define DIGIT_COLOR 3
#define DIGIT_WIDTH	3
#define DIGIT_HEIGHT 5
#define DIGIT_INTEGER_SCALE 1
#define DIGIT_EFFECTIVE_WIDTH (DIGIT_WIDTH * DIGIT_INTEGER_SCALE)
#define DIGIT_EFFECTIVE_HEIGHT (DIGIT_HEIGHT * DIGIT_INTEGER_SCALE)
#define DIGIT_SPACING 2

static void drawValueDigit(dl_u8* framebufferLocation, dl_u8 digit)
{
	int rowLoop;
	int columnLoop;
	int drawLoopX;
	int drawLoopY;
	const dl_u8* digitPattern = g_digitsFont[digit];

	for (rowLoop = 0; rowLoop < DIGIT_HEIGHT; rowLoop++)
	{
		dl_u8 rowBits = *digitPattern;
		digitPattern++;

		for (columnLoop = 0; columnLoop < DIGIT_WIDTH; columnLoop++)
		{
			// go through the bits in the row, highest bit first
			dl_u8 pixelColor = (rowBits & (1 << (2 - columnLoop))) ? DIGIT_COLOR : 0;

			dl_u8* drawLocation = framebufferLocation;

			// draw the pixel scaled
			for (drawLoopY = 0; drawLoopY < DIGIT_INTEGER_SCALE; drawLoopY++)
			{
				for (drawLoopX = 0; drawLoopX < DIGIT_INTEGER_SCALE; drawLoopX++)
				{
					*drawLocation = pixelColor;
					drawLocation++;
				}

				drawLocation += (SCREEN_WIDTH - DIGIT_INTEGER_SCALE);
			}

			// move to the start of the next scaled pixel to draw
			framebufferLocation += DIGIT_INTEGER_SCALE;
		}

		// move to the next line
		framebufferLocation += (SCREEN_WIDTH - (DIGIT_WIDTH * DIGIT_INTEGER_SCALE));
	}
}

void drawValue(dl_u16 x, dl_u16 y, dl_u32 value)
{
	dl_u8 digits[12];
	int digitCount = 0;
	int loop;
	dl_u8* framebuffer = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	if (value == 0)
	{
		digits[digitCount++] = 0;
	}
	else
	{
		while (value)
		{
			digits[digitCount++] = (dl_u8)(value % 10);
			value /= 10;
		}
	}

	for (loop = digitCount - 1; loop >= 0; loop--)
	{
		drawValueDigit(framebuffer, digits[loop]);
		framebuffer += DIGIT_EFFECTIVE_WIDTH + DIGIT_SPACING;
	}
}
