#include "draw_background.h"

#include "base_defines.h"
#include <string.h>

u8 DrawSegment_CurrentScreenX_0x22;
u8 DrawSegment_CurrentScreenY_0x21;

u16 DrawSegment_SecondaryAxisSubpixelInc_0x1e;

u16 D;

void (*g_moveFunction)();
u8 DrawSegmentLine_SubPixelStartValue_Maybe_0x25;

u8 subpixelCount;
u8 pixelCount;

byte* g_framebuffer;

u8 crtMaskIndexToUse;
u8 crtMasks[4] = 
{
	0x00,
	0x55,
	0xaa,
	0xff
};

void DrawSegment_MovePosUpAndRight()
{
	DrawSegment_CurrentScreenY_0x21--;
	D += DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosRightAndUp()
{
	DrawSegment_CurrentScreenX_0x22++;
	D -= DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosRightAndDown()
{
	DrawSegment_CurrentScreenX_0x22++;
	D += DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosDownAndRight()
{
	DrawSegment_CurrentScreenY_0x21++;
	D += DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosDownAndLeft()
{
	DrawSegment_CurrentScreenY_0x21++;
	D -= DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosLeftAndDown()
{
	DrawSegment_CurrentScreenX_0x22--;
	D += DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosLeftAndUp()
{
	DrawSegment_CurrentScreenX_0x22--;
	D -= DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

void DrawSegment_MovePosUpAndLeft()
{
	DrawSegment_CurrentScreenY_0x21--;
	D -= DrawSegment_SecondaryAxisSubpixelInc_0x1e;
}

u8 masks[] = 
{
	0xc0, // 11000000b,
	0x30, // 00110000b,
	0x0C, // 00001100b,
	0x03  // 00000011b,
};


void DrawPixel()
{
	u8 y = DrawSegment_CurrentScreenY_0x21;
	u8 x = DrawSegment_CurrentScreenX_0x22;
	u16 framebufferOffset = (y * FRAMEBUFFER_PITCH) + ((x << 1) / 8);

	u8 maskIndex = x & 0x3;
	u8 maskToUse = masks[maskIndex] & crtMasks[crtMaskIndexToUse]; // apply crt artificat mask

	// remove the pixel (the two bits) at the location and replace with the new two bits

	g_framebuffer[framebufferOffset] = (g_framebuffer[framebufferOffset] & ~masks[maskIndex]) | maskToUse;
}

void DrawHorizontalSegment()
{
	D = (DrawSegment_CurrentScreenY_0x21 << 8) | DrawSegmentLine_SubPixelStartValue_Maybe_0x25;

	while (pixelCount--)
	{
		DrawPixel();

		if (!pixelCount)
			break;

		g_moveFunction();
		DrawSegment_CurrentScreenY_0x21 = D >> 8;
	}
}

void DrawVerticalSegment()
{
	D = (DrawSegment_CurrentScreenX_0x22 << 8) | DrawSegmentLine_SubPixelStartValue_Maybe_0x25;

	while (pixelCount--)
	{
		DrawPixel();

		if (!pixelCount)
			break;

		g_moveFunction();
		DrawSegment_CurrentScreenX_0x22 = D >> 8;
	}

	//DrawSegment_CurrentScreenY_0x21 += 10;
}



void DrawSegment_Orientation0_UpAndRight(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosUpAndRight;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0;
	DrawVerticalSegment();
}

void DrawSegment_Orientation1_RightAndUp(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosRightAndUp;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0xff;
	DrawHorizontalSegment();
}

void DrawSegment_Orientation2_RightAndDown(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosRightAndDown;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0;
	DrawHorizontalSegment();
}

void DrawSegment_Orientation3_DownAndRight(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosDownAndRight;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0;
	DrawVerticalSegment();
}

void DrawSegment_Orientation4_DownAndLeft(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosDownAndLeft;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0xff;
	DrawVerticalSegment();
}

void DrawSegment_Orientation5_LeftAndDown(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosLeftAndDown;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0;
	DrawHorizontalSegment();
}

void DrawSegment_Orientation6_LeftAndUp(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosLeftAndUp;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0xff;
	DrawHorizontalSegment();
}

void DrawSegment_Orientation7_UpAndLeft(u8 subpixelCount, u8 pixelCount)
{
	g_moveFunction = DrawSegment_MovePosUpAndLeft;
	DrawSegmentLine_SubPixelStartValue_Maybe_0x25 = 0xff;
	DrawVerticalSegment();
}

void (*drawSegmentFunctions[])(u8 subpixelCount, u8 pixelCount) = {
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
		subpixelCount = shapeSegmentRunner->subpixelCount;
		DrawSegment_SecondaryAxisSubpixelInc_0x1e = subpixelCount;
		pixelCount = shapeSegmentRunner->pixelCount;
		drawSegmentFunctions[shapeSegmentRunner->orientation](shapeSegmentRunner->subpixelCount,
															  shapeSegmentRunner->pixelCount);
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
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_0c_VeryShortRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0d_ShortRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_0d_ShortRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0e_MidLengthRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_0e_MidLengthRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_0f_LongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_0f_LongRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_10_VeryLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_10_VeryLongRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_11_SuperLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_11_SuperLongRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_12_ExcessivelyLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_12_ExcessivelyLongRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_13_RediculouslyLongRope(const Resources* resources)
{
	DrawPiece(&resources->shapeDrawData_PreRope_Maybe);
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_13_RediculouslyLongRope);
	crtMaskIndexToUse = 1;
	DrawPiece(&resources->shapeDrawData_PostRope_Maybe);
}

void DrawPiece_14_HorizontalRopeStartGoingRight(const Resources* resources)
{
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_14_HorizontalRopeStartGoingRight);
	crtMaskIndexToUse = 1;
}

void DrawPiece_15_HorizontalRopeEndGoingRight(const Resources* resources)
{
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_15_HorizontalRopeEndGoingRight);
	crtMaskIndexToUse = 1;
}

void DrawPiece_16_HorizontalRopeGoingRight(const Resources* resources)
{
	crtMaskIndexToUse = 3;
	DrawPiece(&resources->shapeDrawData_17_BlankAreaGoingRight);
	crtMaskIndexToUse = 1;
}

void DrawPiece_17_BlankAreaGoingRight(const Resources* resources)
{
	crtMaskIndexToUse = 0;
	DrawPiece(&resources->shapeDrawData_17_BlankAreaGoingRight);
	crtMaskIndexToUse = 1;
}

void DrawPiece_18_BlankAreaGoingLeft(const Resources* resources)
{
	crtMaskIndexToUse = 0;
	DrawPiece(&resources->shapeDrawData_18_BlankAreaGoingLeft);
	crtMaskIndexToUse = 1;
}

void DrawPiece_19_BlankAreaGoingDownRight(const Resources* resources)
{
	crtMaskIndexToUse = 0;
	DrawPiece(&resources->shapeDrawData_19_BlankAreaGoingDownRight);
	crtMaskIndexToUse = 1;
}

void DrawPiece_20_UnknownOrBuggy(const Resources* resources)
{
	crtMaskIndexToUse = 0;
	DrawPiece(&resources->shapeDrawData_07_WallPieceGoingUp);
	crtMaskIndexToUse = 1;
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

void Draw_Background(const BackgroundDrawData* backgroundDrawData, 
					 const Resources* resources,
					 byte* framebuffer)
{
	memset(framebuffer, 0, FRAMEBUFFER_SIZE);
	DrawSegment_CurrentScreenX_0x22 = 15;
	DrawSegment_CurrentScreenY_0x21 = 16;

	g_framebuffer = framebuffer;

	u16 D = 0x100f;

	crtMaskIndexToUse = 1;

	int counter = backgroundDrawData->drawCommandCount;
	BackgroundDrawCommand* backgroundDrawCommandRunner = backgroundDrawData->backgroundDrawCommands;

	u8 drawCommandCount = backgroundDrawData->drawCommandCount;

	for (int loop = 0; loop < drawCommandCount; loop++)
	{
		u8 drawPieceCount = backgroundDrawCommandRunner->drawCount;

		for (int shapeLoop = 0; shapeLoop < drawPieceCount; shapeLoop++)
		{
			u8 shapeCode = backgroundDrawCommandRunner->shapeCode;
			drawPieceFunctions[shapeCode](resources);
		}

		counter--;
		backgroundDrawCommandRunner++;
	}

}