#ifndef BACKGROUND_TYPES_INCLUDE_H
#define BACKGROUND_TYPES_INCLUDE_H

#include "base_types.h"

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

#endif 
