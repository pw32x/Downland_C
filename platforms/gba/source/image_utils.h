#ifndef IMAGE_UTILS_INCLUDE_H
#define IMAGE_UTILS_INCLUDE_H

#include "..\..\..\game\base_types.h"

enum CrtColor
{
    CrtColor_Blue,
    CrtColor_Orange
};

void convert1bppImageTo8bppCrtEffectImage(const dl_u8* originalImage,
                                          dl_u8* destinationImage,
                                          dl_u16 width,
                                          dl_u16 height,
                                          enum CrtColor crtColor);

dl_u16 convertToTiles(const dl_u8* sprite, 
					  dl_u8 width,
					  dl_u8 height,
					  int tileIndexStartInBytes);

#endif