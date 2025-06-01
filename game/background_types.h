#ifndef BACKGROUND_TYPES_INCLUDE_H
#define BACKGROUND_TYPES_INCLUDE_H

#include "base_types.h"

typedef struct
{
    u8 subpixelIncrement;
    u8 pixelCount;
    u8 orientation;
} ShapeSegment;

typedef struct
{
    u8 segmentCount;
    const ShapeSegment* segments;
} ShapeDrawData;

typedef struct
{
    u8 shapeCode;
    u8 drawCount;
} BackgroundDrawCommand;

typedef struct
{
    u8 drawCommandCount;
    BackgroundDrawCommand* backgroundDrawCommands;
} BackgroundDrawData;

#endif 
