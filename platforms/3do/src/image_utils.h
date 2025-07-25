#ifndef IMAGE_UTILS_INCLUDE_H
#define IMAGE_UTILS_INCLUDE_H

#include "base_types.h"

enum CrtColor
{
    CrtColor_Blue,
    CrtColor_Orange
};

void convert1bppImageTo2bppCrtEffectImage(const dl_u8* originalImage,
                                          dl_u8* destinationImage,
                                          dl_u16 width,
                                          dl_u16 height,
                                          dl_u16 destinationBufferWidth,
                                          enum CrtColor crtColor);

void convert1bppImageTo2bppBlueImage(const dl_u8* originalImage,
                                     dl_u8* destinationImage,
                                     dl_u16 width,
                                     dl_u16 height,
                                     dl_u16 destinationBufferWidth);


#endif