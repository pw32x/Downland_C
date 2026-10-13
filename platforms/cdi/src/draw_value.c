#include "draw_value.h"
#include "video.h"
#include "graphics.h"
#include "draw_sprite_utils.h"

// small blocky 3x5 retro digit font
static const dl_u8 g_valueDigitFont[10][5] = 
{
	{7, 5, 5, 5, 7}, // 0
	{2, 6, 2, 2, 7}, // 1
	{7, 1, 7, 4, 7}, // 2
	{7, 1, 7, 1, 7}, // 3
	{5, 5, 7, 1, 1}, // 4
	{7, 4, 7, 1, 7}, // 5
	{7, 4, 7, 5, 7}, // 6
	{7, 1, 1, 1, 1}, // 7
	{7, 5, 7, 5, 7}, // 8
	{7, 5, 7, 1, 7}  // 9
};

#define VALUE_DIGIT_SCALE 1
#define VALUE_DIGIT_W (3 * VALUE_DIGIT_SCALE)
#define VALUE_DIGIT_H (5 * VALUE_DIGIT_SCALE)
#define VALUE_DIGIT_GAP 2

static void drawValueDigit(dl_u8* dest, dl_u8 digit)
{
	int row;
	int column;
	int rowDrawCount;
	int columnDrawCount;
	const dl_u8* digitPattern = g_valueDigitFont[digit];

	for (row = 0; row < 5; row++)
	{
		dl_u8 bits = digitPattern[row];
		for (column = 0; column < 3; column++)
		{
			if (bits & (1 << (2 - column)))
			{
				for (rowDrawCount = 0; rowDrawCount < VALUE_DIGIT_SCALE; rowDrawCount++)
				{
					dl_u8* p = dest + (row * VALUE_DIGIT_SCALE + rowDrawCount) * SCREEN_WIDTH + column * VALUE_DIGIT_SCALE;
					for (columnDrawCount = 0; columnDrawCount < VALUE_DIGIT_SCALE; rowDrawCount++)
					{
						p[columnDrawCount] = 3;
					}
				}
			}
		}
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
		framebuffer += VALUE_DIGIT_W + VALUE_DIGIT_GAP;
	}
}
