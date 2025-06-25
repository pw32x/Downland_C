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

void convert1bppImageToVRAM(const dl_u8* originalImage,
                            dl_u16* vram,
                            dl_u16 width,
                            dl_u16 height,
                            enum CrtColor crtColor);

void convertBackgroundToVRAM256(const dl_u8* originalImage,
                                dl_u16* vramTileAddr,
                                dl_u16* vramTileMapAddr,
                                dl_u16 width,
                                dl_u16 height,
                                enum CrtColor crtColor);

dl_u16 convertToTiles(const dl_u8* sprite, 
					  dl_u16 width,
					  dl_u16 height,
                      void* vramLocation,
					  dl_u16 offsetInBytes);

#endif