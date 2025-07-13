#ifndef BACKGROUND_TYPES_INCLUDE_H
#define BACKGROUND_TYPES_INCLUDE_H

#include "base_types.h"

typedef  __packed struct
{
    dl_u8 subpixelIncrement;
    dl_u8 pixelCount;
    dl_u8 orientation;
} ShapeSegment;

typedef __packed struct
{
    dl_u8 segmentCount;
    const ShapeSegment* segments;
} ShapeDrawData;

typedef __packed struct
{
    dl_u8 shapeCode;
    dl_u8 drawCount;
} BackgroundDrawCommand;

typedef __packed struct
{
    dl_u8 drawCommandCount;
    const BackgroundDrawCommand* backgroundDrawCommands;
} BackgroundDrawData;

#endif 
