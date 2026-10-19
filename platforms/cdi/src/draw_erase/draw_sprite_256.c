#include "draw_sprite_256.h"
#include "draw_sprite_256_asm.h"
#include "base_defines.h"


void drawSprite16x16_256(dl_u16 x,
						 dl_u16 y,
						 dl_u8 frameIndex,
						 const GameSprite* gameSprite,
						 dl_u8* buffer)
{
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	buffer += x + (y * FRAMEBUFFER_WIDTH);

	drawRow16x16MaskedFastAsm256(buffer, spriteDataRunner);
}

void drawSprite16x10_256(dl_u16 x,
						 dl_u16 y,
						 dl_u8 frameIndex,
						 const GameSprite* gameSprite,
						 dl_u8* buffer)
{
	const dl_u8* spriteDataRunner = gameSprite->spriteData + (gameSprite->sizePerFrame * frameIndex);
	buffer += x + (y * FRAMEBUFFER_WIDTH);

	drawRow16x10MaskedFastAsm256(buffer, spriteDataRunner);
}

void drawText8_256(dl_u32 xyLocation,
				  dl_u8 characterIndex,
				  const GameSprite* characterFont,
				  dl_u8* buffer)
{
	const dl_u8* spriteDataRunner = characterFont->spriteData + (characterFont->sizePerFrame * characterIndex);
	buffer += xyLocation;

	drawText8x7UnconditionalAsm256(buffer, spriteDataRunner);
}

void drawUIText_256(const dl_u8* text,
				   dl_u32 xyLocation,
				   const GameSprite* characterFont,
				   dl_u8* buffer)
{
	// draw each character, 8 pixels apart
	while (*text != 0xff)
	{
		drawText8_256(xyLocation, *text, characterFont, buffer);
		text++;
		xyLocation += 8;
	}
}
