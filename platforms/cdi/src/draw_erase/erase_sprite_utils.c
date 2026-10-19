#include "erase_sprite_utils.h"
#include "erase_sprite_asm.h"
#include "base_defines.h"
#include "../cdi_hw/graphics.h"

extern dl_u8* g_cleanBackground;

void eraseSprite16(dl_u16 x,
				   dl_u16 y,
				   dl_u8 height)
{
	const dl_u8* cleanBackgroundRunner = g_cleanBackground + (x + (y * FRAMEBUFFER_WIDTH));
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);
	dl_u16 loopy;

	switch (height)
	{
		case 16:
			eraseRestoreRow16x16Asm(frameBufferRunner, cleanBackgroundRunner);
			return;
		case 10:
			eraseRestoreRow16x10Asm(frameBufferRunner, cleanBackgroundRunner);
			return;
		case 8:
			eraseRestoreRow16x8Asm(frameBufferRunner, cleanBackgroundRunner);
			return;
		case 6:
			eraseRestoreRow16x6Asm(frameBufferRunner, cleanBackgroundRunner);
			return;
	}

	for (loopy = 0; loopy < height; loopy++)
	{
		memcpy(frameBufferRunner, cleanBackgroundRunner, 16);

		cleanBackgroundRunner += FRAMEBUFFER_WIDTH;
		frameBufferRunner += SCREEN_WIDTH;
	}
}

void eraseSpriteDrop(dl_u16 x, dl_u16 y, dl_u8 height)
{
	const dl_u8* cleanBackgroundRunner = g_cleanBackground + (x + (y * FRAMEBUFFER_WIDTH));
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	eraseRestoreRow4x6Asm(frameBufferRunner, cleanBackgroundRunner);
}

void eraseSprite24(dl_u16 x,
				 dl_u16 y,
				 dl_u8 height)
{
	dl_u16 loopy;
	const dl_u8* cleanBackgroundRunner = g_cleanBackground + (x + (y * FRAMEBUFFER_WIDTH));
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	for (loopy = 0; loopy < height; loopy++)
	{
		memcpy(frameBufferRunner, cleanBackgroundRunner, 24);

		cleanBackgroundRunner += FRAMEBUFFER_WIDTH;
		frameBufferRunner += SCREEN_WIDTH;
	}
}

void eraseSprite8(dl_u16 x,
				 dl_u16 y,
				 dl_u8 height)
{
	dl_u16 loopy;
	const dl_u8* cleanBackgroundRunner = g_cleanBackground + (x + (y * FRAMEBUFFER_WIDTH));
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	for (loopy = 0; loopy < height; loopy++)
	{
		memcpy(frameBufferRunner, cleanBackgroundRunner, 8);

		cleanBackgroundRunner += FRAMEBUFFER_WIDTH;
		frameBufferRunner += SCREEN_WIDTH;
	}
}
