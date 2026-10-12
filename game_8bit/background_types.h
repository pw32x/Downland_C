#ifndef BACKGROUND_TYPES_INCLUDE_H
#define BACKGROUND_TYPES_INCLUDE_H

#ifdef ENABLE_CLEAN_BACKGROUND_DRAW

#include "base_types.h"
#include "base_defines.h"

typedef struct
{
    dl_u8 subpixelIncrement;
    dl_u8 pixelCount;
    dl_u8 orientation;
} ShapeSegment;

typedef struct
{
    dl_u8 segmentCount;
    const ShapeSegment* segments;
} ShapeDrawData;

typedef struct
{
    dl_u8 shapeCode;
    dl_u8 drawCount;
} BackgroundDrawCommand;

typedef struct
{
    dl_u8 drawCommandCount;
    const BackgroundDrawCommand* backgroundDrawCommands;
} BackgroundDrawData;

typedef struct
{
    // background piece shapes
    const ShapeDrawData* shapeDrawData_00_Stalactite;	                       
    const ShapeDrawData* shapeDrawData_01_WallGoingDown;	                   
    const ShapeDrawData* shapeDrawData_07_WallPieceGoingUp;	               
    const ShapeDrawData* shapeDrawData_02_LeftHandCornerPiece;	               
    const ShapeDrawData* shapeDrawData_08_CornerPieceGoingDownLeft;	       
    const ShapeDrawData* shapeDrawData_03_TopRightHandCornerPiece;	           
    const ShapeDrawData* shapeDrawData_04_TopRightHandCornerPiece2;	       
    const ShapeDrawData* shapeDrawData_05_BottomRightSideOfFloatingPlatforms;
    const ShapeDrawData* shapeDrawData_14_HorizontalRopeStartGoingRight;	   
    const ShapeDrawData* shapeDrawData_15_HorizontalRopeEndGoingRight;	       
    const ShapeDrawData* shapeDrawData_17_BlankAreaGoingRight;	               
    const ShapeDrawData* shapeDrawData_18_BlankAreaGoingLeft;	               
    const ShapeDrawData* shapeDrawData_19_BlankAreaGoingDownRight;	           
    const ShapeDrawData* shapeDrawData_0b_ShortLineGoingUp;	               
    const ShapeDrawData* shapeDrawData_0c_VeryShortRope;	                   
    const ShapeDrawData* shapeDrawData_0d_ShortRope;	                       
    const ShapeDrawData* shapeDrawData_0e_MidLengthRope;	                   
    const ShapeDrawData* shapeDrawData_0f_LongRope;	                       
    const ShapeDrawData* shapeDrawData_10_VeryLongRope;	                   
    const ShapeDrawData* shapeDrawData_11_SuperLongRope;	                   
    const ShapeDrawData* shapeDrawData_12_ExcessivelyLongRope;	               
    const ShapeDrawData* shapeDrawData_13_RediculouslyLongRope;	           
    const ShapeDrawData* shapeDrawData_PreRope_Maybe;	                       
    const ShapeDrawData* shapeDrawData_PostRope_Maybe;
} BackgroundShapeDrawData;

#endif 
#endif