#include "draw_sprite_384.h"
#include "draw_sprite_384_asm.h"
#include "base_defines.h"
#include "cdi_hw/video.h"
#include "cdi_hw/graphics.h"
#include "draw_erase/erase_system.h"
#include "draw_erase/erase_sprite_utils.h"

void drawSprite16_384(dl_u16 x,
					  dl_u16 y,
					  dl_u8 frameIndex,
					  const GameSprite* gameSprite,
					  dl_u8 addToErase)
{
	dl_u32 frameHeight = gameSprite->frameHeight;
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	// draw with the routine that matches the sprite height
	switch (frameHeight)
	{
		case 16:
			drawRow16x16MaskedFastAsm(frameBufferRunner, spriteDataRunner);
			break;
		case 10:
			drawRow16x10MaskedFastAsm(frameBufferRunner, spriteDataRunner);
			break;
		case 8:
			drawRow16x8MaskedFastAsm(frameBufferRunner, spriteDataRunner);
			break;
		case 6:
			drawRow16x6MaskedFastAsm(frameBufferRunner, spriteDataRunner);
			break;
	}

	// remember the sprite so it can be erased
	if (addToErase)
		addToEraseList(x, y, frameHeight, eraseSprite16);
}

void drawSprite16Opaque_384(dl_u16 x,
							dl_u16 y,
							dl_u8 frameIndex,
							const GameSprite* gameSprite)
{
	dl_u32 loopx;
	dl_u32 loopy;
	dl_u32 frameHeight = gameSprite->frameHeight;
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	// draw with the routine that matches the sprite height
	switch (frameHeight)
	{
		case 16:
			drawOpaqueRow16x16Asm(frameBufferRunner, spriteDataRunner);
			return;
		case 10:
			drawOpaqueRow16x10Asm(frameBufferRunner, spriteDataRunner);
			return;
		case 7:
			drawOpaqueRow16x7Asm(frameBufferRunner, spriteDataRunner);
			return;
	}

	// no routine for this height, copy the sprite a pixel at a time
	for (loopy = 0; loopy < frameHeight; loopy++)
	{
		for (loopx = 0; loopx < 16; loopx++)
		{
			*frameBufferRunner = *spriteDataRunner;
			frameBufferRunner++;
			spriteDataRunner++;
		}

		// move to the start of the next line
		frameBufferRunner += (SCREEN_WIDTH - 16);
	}
}

void drawSpriteDrop_384(dl_u16 x,
						dl_u16 y,
						dl_u8 frameIndex,
						const GameSprite* gameSprite)
{
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	drawRow4x6MaskedFastAsm(frameBufferRunner, spriteDataRunner);

	// remember the sprite so it can be erased
	addToEraseList(x, y, gameSprite->frameHeight, eraseSpriteDrop);
}

void drawSprite24_384(dl_u16 x,
					  dl_u16 y,
					  dl_u8 frameIndex,
					  const GameSprite* gameSprite)
{
	dl_u32 loopx;
	dl_u32 loopy;
	dl_u8 pixel;
	dl_u32 frameHeight = gameSprite->frameHeight;
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	// copy the sprite a pixel at a time, skipping transparent pixels
	for (loopy = 0; loopy < frameHeight; loopy++)
	{
		for (loopx = 0; loopx < 24; loopx++)
		{
			pixel = *spriteDataRunner;

			if (pixel)
				*frameBufferRunner = pixel;

			frameBufferRunner++;
			spriteDataRunner++;
		}

		// move to the start of the next line
		frameBufferRunner += (SCREEN_WIDTH - 24);
	}

	// remember the sprite so it can be erased
	addToEraseList(x, y, frameHeight, eraseSprite24);
}

void drawSprite8_384(dl_u16 x,
					 dl_u16 y,
					 dl_u8 frameIndex,
					 const GameSprite* gameSprite)
{
	dl_u32 loopx;
	dl_u32 loopy;
	dl_u8 pixel;
	dl_u32 frameHeight = gameSprite->frameHeight;
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	dl_u8* frameBufferRunner = BUFFER_START + (x + SCREEN_OFFSET_X) + ((y + SCREEN_OFFSET_Y) * SCREEN_WIDTH);

	// copy the sprite a pixel at a time, skipping transparent pixels
	for (loopy = 0; loopy < frameHeight; loopy++)
	{
		for (loopx = 0; loopx < 8; loopx++)
		{
			pixel = *spriteDataRunner;

			if (pixel)
				*frameBufferRunner = pixel;

			frameBufferRunner++;
			spriteDataRunner++;
		}

		// move to the start of the next line
		frameBufferRunner += (SCREEN_WIDTH - 8);
	}

	// remember the sprite so it can be erased
	addToEraseList(x, y, frameHeight, eraseSprite8);
}

void drawText8_384(dl_u32 xyLocation,
				   dl_u8 characterIndex,
				   const GameSprite* characterFont)
{
	const dl_u8* spriteDataRunner = characterFont->spriteData + (characterFont->sizePerFrame * characterIndex);
	dl_u8* frameBufferRunner = BUFFER_START + xyLocation;

	drawText8x7UnconditionalAsm(frameBufferRunner, spriteDataRunner);
}

void drawUIText_384(const dl_u8* text,
				    dl_u32 xyLocation,
				    const GameSprite* characterFont)
{
	// draw each character, 8 pixels apart
	while (*text != 0xff)
	{
		drawText8_384(xyLocation, *text, characterFont);
		text++;
		xyLocation += 8;
	}
}
