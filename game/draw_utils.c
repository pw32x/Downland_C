#include "draw_utils.h"

#include <memory.h>

#include "base_defines.h"

void setPixel(u8* framebuffer, u8 x, u8 y, u8 value) 
{
    if (x < 0 || x >= FRAMEBUFFER_WIDTH || y < 0 || y >= FRAMEBUFFER_HEIGHT) 
        return;

    u8 pixel = 1 << (7 - (x % 8));
    int index = (x / 8) + (y * FRAMEBUFFER_PITCH);

    if (value)
        framebuffer[index] |= pixel;  // set bit/pixel
    else
        framebuffer[index] &= ~pixel; // clear bit/pixel
}

void drawText(u8* text, u8* characterFont, u8* framebuffer, u16 framebufferPosition)
{
    u8 rowsPerCharacter = 7;
    framebuffer += framebufferPosition;

    // for each character
    while (*text != 0xff)
    {
        // find the corresponding character in the font
        u8* character = &characterFont[*text * rowsPerCharacter]; // index of the character * 7 bytes per character if font

        for (int loop = 0; loop < rowsPerCharacter; loop++)
        {
            *framebuffer = character[loop] & CRT_EFFECT_MASK;
            framebuffer += 0x20; // go down one row in the frame buffer for the next line.
        }

        framebuffer -= 0xdf;

        text++;
    }
}

void drawSprite_16PixelsWide(u8* sprite, 
                             u8 x, 
                             u8 y, 
                             u8 numLines,
                             u8* framebuffer)
{
    framebuffer += (x / 4) + (y * FRAMEBUFFER_PITCH);

    // for each character
    while (numLines--)
    {
        // first byte
        *framebuffer |= *sprite;
        framebuffer++;
        sprite++;

        // second byte
        *framebuffer |= *sprite;
        sprite++;

        // move framebuffer to next row
        framebuffer += (FRAMEBUFFER_PITCH - 1); // go down one row in the frame buffer for the next line.
    }
}


void eraseSprite_16PixelsWide(u8* framebuffer, 
							  u8* cleanBackground,
							  u8 x,
							  u8 y,
							  u8* spriteData, 
							  u8 rowCount)
{
	u16 offset = (x / 4) + (y * FRAMEBUFFER_PITCH);
	framebuffer += offset;
	cleanBackground += offset;

	for (int loop = 0; loop < rowCount; loop++)
	{
		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;
		framebuffer++;
		cleanBackground++;

		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;

		framebuffer += (FRAMEBUFFER_PITCH - 1); // move to the next row
		cleanBackground += (FRAMEBUFFER_PITCH - 1);
	}
}

/*
// keeping these just in case
void eraseSprite(u8* framebuffer, 
				 u8* cleanBackground,
				 u16 framebufferDrawLocation, 
				 u8* spriteData, 
				 u8 rowCount)
{
	framebuffer += framebufferDrawLocation;
	cleanBackground += framebufferDrawLocation;

	for (int loop = 0; loop < rowCount; loop++)
	{
		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;
		framebuffer++;
		cleanBackground++;

		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;

		framebuffer += (FRAMEBUFFER_PITCH - 1); // move to the next row
		cleanBackground += (FRAMEBUFFER_PITCH - 1);
	}
}

void drawSprite(u8* framebuffer, 
				u16 framebufferDrawLocation, 
				u8* spriteData, 
				u8 rowCount)
{
	framebuffer += framebufferDrawLocation;

	for (int loop = 0; loop < rowCount; loop++)
	{
		// blend the first byte of sprite's pixels with the frame buffer
		*framebuffer |= *spriteData;
		spriteData++;
		framebuffer++;

		// blend the second byte of sprite's pixels with the frame buffer
		*framebuffer |= *spriteData;
		spriteData++;

		framebuffer += (FRAMEBUFFER_PITCH - 1); // move to the next row
	}
}
*/


u8* g_framebuffer;

u8 g_plotterCurrentY;
u8 g_plotterCurrentX;
u16 g_plotterHiresPos;

u8 g_subpixelInitValue;
u8 g_pixelCount;
u16 g_subpixelIncrement;

u8 g_crtMaskIndexToUse;

void DrawSegment_MovePlotterUpAndRight()
{
	g_plotterCurrentX--;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterRightAndUp()
{
	g_plotterCurrentY++;
	g_plotterHiresPos -= g_subpixelIncrement;
}

void DrawSegment_MovePlotterRightAndDown()
{
	g_plotterCurrentY++;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterDownAndRight()
{
	g_plotterCurrentX++;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterDownAndLeft()
{
	g_plotterCurrentX++;
	g_plotterHiresPos -= g_subpixelIncrement;
}

void DrawSegment_MovePlotterLeftAndDown()
{
	g_plotterCurrentY--;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterLeftAndUp()
{
	g_plotterCurrentY--;
	g_plotterHiresPos -= g_subpixelIncrement;
}

void DrawSegment_MovePlotterUpAndLeft()
{
	g_plotterCurrentX--;
	g_plotterHiresPos -= g_subpixelIncrement;
}

// depending on the x position, chose 
// of these to get at the corresponding
// two bits you want to activate.
u8 pixelMasks[4] = 
{
	0xc0, // 11000000b,
	0x30, // 00110000b,
	0x0C, // 00001100b,
	0x03  // 00000011b,
};

// different pixel masks to acheive different pixel
// colors using crt artifact effects.
u8 crtMasks[4] = 
{
	0x00, // 00000000b - black
	0x55, // 01010101b - blue
	0xaa, // 10101010b - orange
	0xff  // 11111111b - white
};

enum 
{
	CRT_MASK_INVISIBLE,
	CRT_MASK_NORMAL,
	CRT_MASK_REVERSE,
	CRT_MASK_FULL
} CRT_MASKS;

void DrawPixel()
{
	u8 y = g_plotterCurrentX;
	u8 x = g_plotterCurrentY;
	u8* framebufferPos = g_framebuffer + (y * FRAMEBUFFER_PITCH) + ((x << 1) / 8);

	u8 pixelMaskIndex = x & 0x3;
	u8 pixelMask = pixelMasks[pixelMaskIndex];
	u8 pixelToDraw = pixelMask & crtMasks[g_crtMaskIndexToUse]; // apply crt artifact mask on top of the pixel

	if (!pixelToDraw)
		return;

	// remove the pixel (the two bits) at the location and replace with the new two bits
	*framebufferPos = (*framebufferPos & ~pixelMask) | pixelToDraw;
}

void DrawHorizontalSegment(void (*movePlotterFunction)())
{
	g_plotterHiresPos = SET_HIRES(g_plotterCurrentX) | g_subpixelInitValue;

	while (g_pixelCount--)
	{
		DrawPixel();

		if (!g_pixelCount)
			break;

		movePlotterFunction();
		g_plotterCurrentX = GET_FROM_HIRES(g_plotterHiresPos);
	}
}

void DrawVerticalSegment(void (*movePlotterFunction)())
{
	g_plotterHiresPos = SET_HIRES(g_plotterCurrentY) | g_subpixelInitValue;

	while (g_pixelCount--)
	{
		DrawPixel();

		if (!g_pixelCount)
			break;

		movePlotterFunction();
		g_plotterCurrentY = GET_FROM_HIRES(g_plotterHiresPos);
	}
}

void DrawSegment_Orientation0_UpAndRight()
{
	g_subpixelInitValue = 0;
	DrawVerticalSegment(DrawSegment_MovePlotterUpAndRight);
}

void DrawSegment_Orientation1_RightAndUp()
{
	g_subpixelInitValue = 0xff;
	DrawHorizontalSegment(DrawSegment_MovePlotterRightAndUp);
}

void DrawSegment_Orientation2_RightAndDown()
{
	g_subpixelInitValue = 0;
	DrawHorizontalSegment(DrawSegment_MovePlotterRightAndDown);
}

void DrawSegment_Orientation3_DownAndRight()
{
	g_subpixelInitValue = 0;
	DrawVerticalSegment(DrawSegment_MovePlotterDownAndRight);
}

void DrawSegment_Orientation4_DownAndLeft()
{
	g_subpixelInitValue = 0xff;
	DrawVerticalSegment(DrawSegment_MovePlotterDownAndLeft);
}

void DrawSegment_Orientation5_LeftAndDown()
{
	g_subpixelInitValue = 0;
	DrawHorizontalSegment(DrawSegment_MovePlotterLeftAndDown);
}

void DrawSegment_Orientation6_LeftAndUp()
{
	g_subpixelInitValue = 0xff;
	DrawHorizontalSegment(DrawSegment_MovePlotterLeftAndUp);
}

void DrawSegment_Orientation7_UpAndLeft()
{
	g_subpixelInitValue = 0xff;
	DrawVerticalSegment(DrawSegment_MovePlotterUpAndLeft);
}

void (*drawSegmentFunctions[])() = {
	DrawSegment_Orientation0_UpAndRight,
	DrawSegment_Orientation1_RightAndUp,
	DrawSegment_Orientation2_RightAndDown,
	DrawSegment_Orientation3_DownAndRight,
	DrawSegment_Orientation4_DownAndLeft,
	DrawSegment_Orientation5_LeftAndDown,
	DrawSegment_Orientation6_LeftAndUp,
	DrawSegment_Orientation7_UpAndLeft,
};
	


void DrawPiece(const ShapeDrawData* shapeDrawData)
{
	u8 count = shapeDrawData->segmentCount;
	const ShapeSegment* shapeSegmentRunner = shapeDrawData->segments;

	while (count--)
	{
		g_subpixelIncrement = shapeSegmentRunner->subpixelIncrement;
		g_pixelCount = shapeSegmentRunner->pixelCount;

		drawSegmentFunctions[shapeSegmentRunner->orientation]();
		shapeSegmentRunner++;
	}
}

void DrawPiece_00_Stalactite(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_00_Stalactite);
}

void DrawPiece_01_WallGoingDown(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_01_WallGoingDown);
}

void DrawPiece_02_LeftHandCornerPiece(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_02_LeftHandCornerPiece);
}

void DrawPiece_03_TopRightHandCornerPiece(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_03_TopRightHandCornerPiece);
}

void DrawPiece_04_TopRightHandCornerPiece2(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_04_TopRightHandCornerPiece2);
}

void DrawPiece_05_BottomRightSideOfFloatingPlatforms(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_05_BottomRightSideOfFloatingPlatforms);
}

void DrawPiece_06_FloorPieceGoingRight(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_17_BlankAreaGoingRight);
}

void DrawPiece_07_WallPieceGoingUp(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_07_WallPieceGoingUp);
}

void DrawPiece_08_CornerPieceGoingDownLeft(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_08_CornerPieceGoingDownLeft);
}

void DrawPiece_09_FloorPieceGoingLeft(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_18_BlankAreaGoingLeft);
}

void DrawPiece_0a_ShortLineGoingDown(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_19_BlankAreaGoingDownRight);
}

void DrawPiece_0b_ShortLineGoingUp(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_0b_ShortLineGoingUp);
}

void DrawPiece_0c_VeryShortRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_0c_VeryShortRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0d_ShortRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_0d_ShortRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0e_MidLengthRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_0e_MidLengthRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0f_LongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_0f_LongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_10_VeryLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_10_VeryLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_11_SuperLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_11_SuperLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_12_ExcessivelyLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_12_ExcessivelyLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_13_RediculouslyLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_13_RediculouslyLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_14_HorizontalRopeStartGoingRight(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_14_HorizontalRopeStartGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_15_HorizontalRopeEndGoingRight(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_15_HorizontalRopeEndGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_16_HorizontalRopeGoingRight(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(&resources->shapeDrawData_17_BlankAreaGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_17_BlankAreaGoingRight(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(&resources->shapeDrawData_17_BlankAreaGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_18_BlankAreaGoingLeft(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(&resources->shapeDrawData_18_BlankAreaGoingLeft);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_19_BlankAreaGoingDownRight(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(&resources->shapeDrawData_19_BlankAreaGoingDownRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_20_UnknownOrBuggy(const Resources* resources)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(&resources->shapeDrawData_07_WallPieceGoingUp);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void (*drawPieceFunctions[])(const Resources* resources) = {
	
	DrawPiece_00_Stalactite, 
	DrawPiece_01_WallGoingDown, 
	DrawPiece_02_LeftHandCornerPiece, 
	DrawPiece_03_TopRightHandCornerPiece, 
	DrawPiece_04_TopRightHandCornerPiece2, 
	DrawPiece_05_BottomRightSideOfFloatingPlatforms, 
	DrawPiece_06_FloorPieceGoingRight, 
	DrawPiece_07_WallPieceGoingUp, 
	DrawPiece_08_CornerPieceGoingDownLeft,
	DrawPiece_09_FloorPieceGoingLeft, 
	DrawPiece_0a_ShortLineGoingDown, 
	DrawPiece_0b_ShortLineGoingUp, 
	DrawPiece_0c_VeryShortRope, 
	DrawPiece_0d_ShortRope, 
	DrawPiece_0e_MidLengthRope, 
	DrawPiece_0f_LongRope, 
	DrawPiece_10_VeryLongRope, 
	DrawPiece_11_SuperLongRope, 
	DrawPiece_12_ExcessivelyLongRope, 
	DrawPiece_13_RediculouslyLongRope, 
	DrawPiece_14_HorizontalRopeStartGoingRight, 
	DrawPiece_15_HorizontalRopeEndGoingRight, 
	DrawPiece_16_HorizontalRopeGoingRight,
	DrawPiece_17_BlankAreaGoingRight, 
	DrawPiece_18_BlankAreaGoingLeft, 
	DrawPiece_19_BlankAreaGoingDownRight, 
	DrawPiece_20_UnknownOrBuggy, 
};

void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const Resources* resources,
					u8* framebuffer)
{
	g_framebuffer = framebuffer;

	// clear frame buffer
	memset(g_framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);

	// init plotter position
	g_plotterCurrentY = 15;
	g_plotterCurrentX = 16;
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;

	int counter = backgroundDrawData->drawCommandCount;
	BackgroundDrawCommand* backgroundDrawCommandRunner = backgroundDrawData->backgroundDrawCommands;

	while (counter--)
	{
		for (int shapeLoop = 0; shapeLoop < backgroundDrawCommandRunner->drawCount; shapeLoop++)
		{
			u8 shapeCode = backgroundDrawCommandRunner->shapeCode;
			drawPieceFunctions[shapeCode](resources);
		}

		backgroundDrawCommandRunner++;
	}
}