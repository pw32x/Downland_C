#include "draw_utils.h"

#include "base_defines.h"
#include "dl_rand.h"
#include "dl_platform.h"

void setPixel(dl_u8* framebuffer, dl_s16 x, dl_s16 y, dl_u8 value) 
{
	dl_u8 pixel;
	int index;

    if (x < 0 || x >= FRAMEBUFFER_WIDTH || y < 0 || y >= FRAMEBUFFER_HEIGHT) 
        return;

    pixel = 1 << (7 - (x % 8));
    index = (x / 8) + (y * FRAMEBUFFER_PITCH);

    if (value)
        framebuffer[index] |= pixel;  // set bit/pixel
    else
        framebuffer[index] &= ~pixel; // clear bit/pixel
}

void drawText(const dl_u8* text, const dl_u8* characterFont, dl_u8* framebuffer, dl_u16 framebufferPosition)
{
	const dl_u8* character;
	int loop;

    dl_u8 rowsPerCharacter = 7;
    framebuffer += framebufferPosition;

    // for each character
    while (*text != 0xff)
    {
        // find the corresponding character in the font
        character = &characterFont[*text * rowsPerCharacter]; // index of the character * 7 bytes per character if font

        for (loop = 0; loop < rowsPerCharacter; loop++)
        {
            *framebuffer = character[loop] & CRT_EFFECT_MASK;
            framebuffer += 0x20; // go down one row in the frame buffer for the next line.
        }

        framebuffer -= 0xdf;

        text++;
    }
}

void drawSprite_16PixelsWide(const dl_u8* spriteData, 
                             dl_u8 x, 
                             dl_u8 y, 
                             dl_u8 numLines,
                             dl_u8* framebuffer)
{
    framebuffer += (x / 4) + (y * FRAMEBUFFER_PITCH);

    // for each character
    while (numLines--)
    {
        // first byte
        *framebuffer |= *spriteData;
        framebuffer++;
        spriteData++;

        // second byte
        *framebuffer |= *spriteData;
        spriteData++;

        // move framebuffer to next row
        framebuffer += (FRAMEBUFFER_PITCH - 1); // go down one row in the frame buffer for the next line.
    }
}

void drawSprite_24PixelsWide(const dl_u8* spriteData, 
                             dl_u8 x, 
                             dl_u8 y, 
                             dl_u8 numLines,
                             dl_u8* framebuffer)
{
    framebuffer += (x / 4) + (y * FRAMEBUFFER_PITCH);

    // for each character
    while (numLines--)
    {
        // first byte
        *framebuffer |= *spriteData;
        framebuffer++;
        spriteData++;

        // second byte
        *framebuffer |= *spriteData;
        framebuffer++;
        spriteData++;

        // third byte
        *framebuffer |= *spriteData;
        spriteData++;

        // move framebuffer to next row
        framebuffer += (FRAMEBUFFER_PITCH - 2); // go down one row in the frame buffer for the next line.
    }
}

void drawSprite_24PixelsWide_noblend(const dl_u8* spriteData, 
									 dl_u8 x, 
									 dl_u8 y, 
									 dl_u8 numLines,
									 dl_u8* framebuffer)
{
    framebuffer += (x / 4) + (y * FRAMEBUFFER_PITCH);

    // for each character
    while (numLines--)
    {
        // first byte
        *framebuffer = *spriteData;
        framebuffer++;
        spriteData++;

        // second byte
        *framebuffer = *spriteData;
        framebuffer++;
        spriteData++;

        // third byte
        *framebuffer = *spriteData;
        spriteData++;

        // move framebuffer to next row
        framebuffer += (FRAMEBUFFER_PITCH - 2); // go down one row in the frame buffer for the next line.
    }
}

dl_u8 corruptByte(dl_u8 value)
{
	return ((value << 1) | value) & (dl_rand() % 0xff);
}

void drawSprite_24PixelsWide_static(const dl_u8* spriteData, 
									dl_u8 x, 
									dl_u8 y, 
									dl_u8 numLines,
									dl_u8* framebuffer)
{
    framebuffer += (x / 4) + (y * FRAMEBUFFER_PITCH);

    // for each character
    while (numLines--)
    {
        // first byte
        *framebuffer |= corruptByte(*spriteData);
        framebuffer++;
        spriteData++;

        // second byte
        *framebuffer |= corruptByte(*spriteData);
        framebuffer++;
        spriteData++;

        // third byte
        *framebuffer |= corruptByte(*spriteData);
        spriteData++;

        // move framebuffer to next row
        framebuffer += (FRAMEBUFFER_PITCH - 2); // go down one row in the frame buffer for the next line.
    }
}


void eraseSprite_16PixelsWide(const dl_u8* spriteData, 
							  dl_u8 x, 
							  dl_u8 y, 
							  dl_u8 numLines, 
							  dl_u8* framebuffer, 
							  dl_u8* cleanBackground)
{
	dl_u16 offset = (x / 4) + (y * FRAMEBUFFER_PITCH);
	int loop;

	framebuffer += offset;
	cleanBackground += offset;

	for (loop = 0; loop < numLines; loop++)
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


void eraseSprite_16PixelsWide_simple(dl_u8 x, 
									 dl_u8 y, 
									 dl_u8 numLines, 
									 dl_u8* framebuffer, 
									 dl_u8* cleanBackground)
{
	dl_u16 offset = (x / 4) + (y * FRAMEBUFFER_PITCH);
	int loop;

	framebuffer += offset;
	cleanBackground += offset;

	for (loop = 0; loop < numLines; loop++)
	{
		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer = *cleanBackground;
		framebuffer++;
		cleanBackground++;

		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer = *cleanBackground;

		framebuffer += (FRAMEBUFFER_PITCH - 1); // move to the next row
		cleanBackground += (FRAMEBUFFER_PITCH - 1);
	}
}

void eraseSprite_24PixelsWide(const dl_u8* spriteData, 
							  dl_u8 x, 
							  dl_u8 y, 
							  dl_u8 numLines, 
							  dl_u8* framebuffer, 
							  dl_u8* cleanBackground)
{
	dl_u16 offset = (x / 4) + (y * FRAMEBUFFER_PITCH);
	int loop;

	framebuffer += offset;
	cleanBackground += offset;

	for (loop = 0; loop < numLines; loop++)
	{
		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;
		framebuffer++;
		cleanBackground++;

		// second byte
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;
		framebuffer++;
		cleanBackground++;

		// third byte
		*framebuffer &= ~(*spriteData);
		*framebuffer |= *cleanBackground;
		spriteData++;

		framebuffer += (FRAMEBUFFER_PITCH - 2); // move to the next row
		cleanBackground += (FRAMEBUFFER_PITCH - 2);
	}
}

void eraseSprite_24PixelsWide_simple(dl_u8 x, 
									 dl_u8 y, 
									 dl_u8 numLines, 
									 dl_u8* framebuffer, 
									 dl_u8* cleanBackground)
{
	dl_u16 offset = (x / 4) + (y * FRAMEBUFFER_PITCH);
	int loop;

	framebuffer += offset;
	cleanBackground += offset;

	for (loop = 0; loop < numLines; loop++)
	{
		// remove the bits of the sprite from the frame buffer 
		// and restore with the clean background
		*framebuffer = *cleanBackground;
		framebuffer++;
		cleanBackground++;

		// second byte
		*framebuffer = *cleanBackground;
		framebuffer++;
		cleanBackground++;

		// third byte
		*framebuffer = *cleanBackground;

		framebuffer += (FRAMEBUFFER_PITCH - 2); // move to the next row
		cleanBackground += (FRAMEBUFFER_PITCH - 2);
	}
}

const dl_u8* getBitShiftedSprite(const dl_u8* bitShiftedSpriteData, dl_u8 frameNumber, dl_u8 x, dl_u8 spriteFrameSize)
{
	// x will be 0 to 3
	return bitShiftedSpriteData + (frameNumber * (spriteFrameSize * 4)) + (x * spriteFrameSize);
}

/*
// keeping these just in case
void eraseSprite(dl_u8* framebuffer, 
				 dl_u8* cleanBackground,
				 dl_u16 framebufferDrawLocation, 
				 const dl_u8* spriteData, 
				 dl_u8 rowCount)
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

void drawSprite(dl_u8* framebuffer, 
				dl_u16 framebufferDrawLocation, 
				const dl_u8* spriteData, 
				dl_u8 rowCount)
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

/*
dl_u8* g_framebuffer;

dl_u8 g_plotterCurrentY;
dl_u8 g_plotterCurrentX;
dl_u16 g_plotterHiresPos;

dl_u8 g_subpixelInitValue;
dl_u8 g_pixelCount;
dl_u16 g_subpixelIncrement;

dl_u8 g_crtMaskIndexToUse;

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
*/
// depending on the x position, chose 
// of these to get at the corresponding
// two bits you want to activate.
dl_u8 pixelMasks[4] = 
{
	0xc0, // 11000000b,
	0x30, // 00110000b,
	0x0C, // 00001100b,
	0x03  // 00000011b,
};

// different pixel masks to acheive different pixel
// colors using crt artifact effects.
dl_u8 crtMasks[4] = 
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
/*
void DrawPixel()
{
	dl_u8 y = g_plotterCurrentX;
	dl_u8 x = g_plotterCurrentY;
	dl_u8* framebufferPos = g_framebuffer + (y * FRAMEBUFFER_PITCH) + ((x << 1) / 8);

	dl_u8 pixelMaskIndex = x & 0x3;
	dl_u8 pixelMask = pixelMasks[pixelMaskIndex];
	dl_u8 pixelToDraw = pixelMask & crtMasks[g_crtMaskIndexToUse]; // apply crt artifact mask on top of the pixel

	if (!pixelToDraw)
		return;

	// remove the pixel (the two bits) at the location and replace with the new two bits
	*framebufferPos = (*framebufferPos & ~pixelMask) | pixelToDraw;
}

void DrawHorizontalSegment(void (*movePlotterFunction)())
{
	g_plotterHiresPos = SET_HIGH_BYTE(g_plotterCurrentX) | g_subpixelInitValue;

	while (g_pixelCount--)
	{
		DrawPixel();

		if (!g_pixelCount)
			break;

		movePlotterFunction();
		g_plotterCurrentX = GET_HIGH_BYTE(g_plotterHiresPos);
	}
}

void DrawVerticalSegment(void (*movePlotterFunction)())
{
	g_plotterHiresPos = SET_HIGH_BYTE(g_plotterCurrentY) | g_subpixelInitValue;

	while (g_pixelCount--)
	{
		DrawPixel();

		if (!g_pixelCount)
			break;

		movePlotterFunction();
		g_plotterCurrentY = GET_HIGH_BYTE(g_plotterHiresPos);
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
	dl_u8 count = shapeDrawData->segmentCount;
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
*/
void drawBackground(const BackgroundDrawData* backgroundDrawData, 
					const Resources* resources,
					dl_u8* framebuffer)
{
	/*
	int counter;
	int shapeLoop;
	const BackgroundDrawCommand* backgroundDrawCommandRunner;

	g_framebuffer = framebuffer;

	// clear frame buffer
	dl_memset(g_framebuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);

	// init plotter position
	g_plotterCurrentY = 15;
	g_plotterCurrentX = 16;
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;

	counter = backgroundDrawData->drawCommandCount;
	backgroundDrawCommandRunner = backgroundDrawData->backgroundDrawCommands;

	while (counter--)
	{
		for (shapeLoop = 0; shapeLoop < backgroundDrawCommandRunner->drawCount; shapeLoop++)
		{
			dl_u8 shapeCode = backgroundDrawCommandRunner->shapeCode;
			drawPieceFunctions[shapeCode](resources);
		}

		backgroundDrawCommandRunner++;
	}
	*/
}

void drawSprite_16PixelsWide_static_IntoSpriteBuffer(const dl_u8* sourceSprite, dl_u8 numLines,	dl_u8* destinationSprite)
{
    while (numLines--)
    {
        // first byte
        *destinationSprite |= corruptByte(*sourceSprite);
        destinationSprite++;
        sourceSprite++;

        // second byte
        *destinationSprite |= corruptByte(*sourceSprite);
        destinationSprite++;
        sourceSprite++;
    }
}