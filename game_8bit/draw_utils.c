#include "draw_utils.h"

#include "base_defines.h"
#include "dl_platform.h"

// depending on the x position, chose 
// of these to get at the corresponding
// two bits you want to activate.
const dl_u8 pixelMasks[4] = 
{
	0xc0, // 11000000b,
	0x30, // 00110000b,
	0x0C, // 00001100b,
	0x03  // 00000011b,
};

#ifdef ENABLE_CLEAN_BACKGROUND_DRAW

#define CRT_EFFECT_MASK 0x55 // 01010101b

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

dl_u8* g_backgroundBuffer;

dl_u8 g_plotterCurrentY;
dl_u8 g_plotterCurrentX;
dl_u16 g_plotterHiresPos;

dl_u8 g_subpixelInitValue;
dl_u8 g_pixelCount;
dl_u16 g_subpixelIncrement;

dl_u8 g_crtMaskIndexToUse;

void DrawSegment_MovePlotterUpAndRight(void)
{
	g_plotterCurrentX--;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterRightAndUp(void)
{
	g_plotterCurrentY++;
	g_plotterHiresPos -= g_subpixelIncrement;
}

void DrawSegment_MovePlotterRightAndDown(void)
{
	g_plotterCurrentY++;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterDownAndRight(void)
{
	g_plotterCurrentX++;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterDownAndLeft(void)
{
	g_plotterCurrentX++;
	g_plotterHiresPos -= g_subpixelIncrement;
}

void DrawSegment_MovePlotterLeftAndDown(void)
{
	g_plotterCurrentY--;
	g_plotterHiresPos += g_subpixelIncrement;
}

void DrawSegment_MovePlotterLeftAndUp(void)
{
	g_plotterCurrentY--;
	g_plotterHiresPos -= g_subpixelIncrement;
}

void DrawSegment_MovePlotterUpAndLeft(void)
{
	g_plotterCurrentX--;
	g_plotterHiresPos -= g_subpixelIncrement;
}


// different pixel masks to acheive different pixel
// colors using crt artifact effects.
const dl_u8 crtMasks[4] = 
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

void DrawPixel(void)
{
	dl_u8 y = g_plotterCurrentX;
	dl_u8 x = g_plotterCurrentY;
	dl_u8* framebufferPos = g_backgroundBuffer + (y * FRAMEBUFFER_PITCH) + ((x << 1) / 8);

	dl_u8 pixelMaskIndex = x & 0x3;
	dl_u8 pixelMask = pixelMasks[pixelMaskIndex];
	dl_u8 pixelToDraw = pixelMask & crtMasks[g_crtMaskIndexToUse]; // apply crt artifact mask on top of the pixel

	if (!pixelToDraw)
		return;

	// remove the pixel (the two bits) at the location and replace with the new two bits
	*framebufferPos = (*framebufferPos & ~pixelMask) | pixelToDraw;
}

void DrawHorizontalSegment(void (*movePlotterFunction)(void))
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

void DrawVerticalSegment(void (*movePlotterFunction)(void))
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

void DrawSegment_Orientation0_UpAndRight(void)
{
	g_subpixelInitValue = 0;
	DrawVerticalSegment(DrawSegment_MovePlotterUpAndRight);
}

void DrawSegment_Orientation1_RightAndUp(void)
{
	g_subpixelInitValue = 0xff;
	DrawHorizontalSegment(DrawSegment_MovePlotterRightAndUp);
}

void DrawSegment_Orientation2_RightAndDown(void)
{
	g_subpixelInitValue = 0;
	DrawHorizontalSegment(DrawSegment_MovePlotterRightAndDown);
}

void DrawSegment_Orientation3_DownAndRight(void)
{
	g_subpixelInitValue = 0;
	DrawVerticalSegment(DrawSegment_MovePlotterDownAndRight);
}

void DrawSegment_Orientation4_DownAndLeft(void)
{
	g_subpixelInitValue = 0xff;
	DrawVerticalSegment(DrawSegment_MovePlotterDownAndLeft);
}

void DrawSegment_Orientation5_LeftAndDown(void)
{
	g_subpixelInitValue = 0;
	DrawHorizontalSegment(DrawSegment_MovePlotterLeftAndDown);
}

void DrawSegment_Orientation6_LeftAndUp(void)
{
	g_subpixelInitValue = 0xff;
	DrawHorizontalSegment(DrawSegment_MovePlotterLeftAndUp);
}

void DrawSegment_Orientation7_UpAndLeft(void)
{
	g_subpixelInitValue = 0xff;
	DrawVerticalSegment(DrawSegment_MovePlotterUpAndLeft);
}

void (*drawSegmentFunctions[])(void) = {
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

void DrawPiece_00_Stalactite(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_00_Stalactite);
}

void DrawPiece_01_WallGoingDown(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_01_WallGoingDown);
}

void DrawPiece_02_LeftHandCornerPiece(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_02_LeftHandCornerPiece);
}

void DrawPiece_03_TopRightHandCornerPiece(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_03_TopRightHandCornerPiece);
}

void DrawPiece_04_TopRightHandCornerPiece2(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_04_TopRightHandCornerPiece2);
}

void DrawPiece_05_BottomRightSideOfFloatingPlatforms(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_05_BottomRightSideOfFloatingPlatforms);
}

void DrawPiece_06_FloorPieceGoingRight(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_17_BlankAreaGoingRight);
}

void DrawPiece_07_WallPieceGoingUp(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_07_WallPieceGoingUp);
}

void DrawPiece_08_CornerPieceGoingDownLeft(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_08_CornerPieceGoingDownLeft);
}

void DrawPiece_09_FloorPieceGoingLeft(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_18_BlankAreaGoingLeft);
}

void DrawPiece_0a_ShortLineGoingDown(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_19_BlankAreaGoingDownRight);
}

void DrawPiece_0b_ShortLineGoingUp(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_0b_ShortLineGoingUp);
}

void DrawPiece_0c_VeryShortRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_0c_VeryShortRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0d_ShortRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_0d_ShortRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0e_MidLengthRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_0e_MidLengthRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0f_LongRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_0f_LongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_10_VeryLongRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_10_VeryLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_11_SuperLongRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_11_SuperLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_12_ExcessivelyLongRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_12_ExcessivelyLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_13_RediculouslyLongRope(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PreRope_Maybe);
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_13_RediculouslyLongRope);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_14_HorizontalRopeStartGoingRight(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_14_HorizontalRopeStartGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_15_HorizontalRopeEndGoingRight(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_15_HorizontalRopeEndGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_16_HorizontalRopeGoingRight(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_FULL;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_17_BlankAreaGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_17_BlankAreaGoingRight(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_17_BlankAreaGoingRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_18_BlankAreaGoingLeft(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_18_BlankAreaGoingLeft);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_19_BlankAreaGoingDownRight(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_19_BlankAreaGoingDownRight);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void DrawPiece_20_UnknownOrBuggy(const BackgroundShapeDrawData* backgroundshapedrawdata)
{
	g_crtMaskIndexToUse = CRT_MASK_INVISIBLE;
	DrawPiece(backgroundshapedrawdata->shapeDrawData_07_WallPieceGoingUp);
	g_crtMaskIndexToUse = CRT_MASK_NORMAL;
}

void (*drawPieceFunctions[])(const BackgroundShapeDrawData* backgroundshapedrawdata) = {
	
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
					const BackgroundShapeDrawData* backgroundshapedrawdata,
					dl_u8* backgroundBuffer)
{
	int counter;
	int shapeLoop;
	const BackgroundDrawCommand* backgroundDrawCommandRunner;

	g_backgroundBuffer = backgroundBuffer;

	// clear frame buffer
	dl_memset(g_backgroundBuffer, 0, FRAMEBUFFER_SIZE_IN_BYTES);

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
			drawPieceFunctions[shapeCode](backgroundshapedrawdata);
		}

		backgroundDrawCommandRunner++;
	}
}
#endif