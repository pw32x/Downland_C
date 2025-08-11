#ifndef BACKGROUND_TYPES_INCLUDE_H
#define BACKGROUND_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

typedef DL_PACKED struct
{
    dl_u8 subpixelIncrement;
    dl_u8 pixelCount;
    dl_u8 orientation;
} ShapeSegment;

typedef DL_PACKED struct
{
    dl_u8 segmentCount;
    const ShapeSegment* segments;
} ShapeDrawData;

typedef DL_PACKED struct
{
    dl_u8 shapeCode;
    dl_u8 drawCount;
} BackgroundDrawCommand;

typedef DL_PACKED struct
{
    dl_u8 drawCommandCount;
    const BackgroundDrawCommand* backgroundDrawCommands;
} BackgroundDrawData;

#endif 
