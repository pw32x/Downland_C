#ifndef IMAGE_UTILS_INCLUDE_H
#define IMAGE_UTILS_INCLUDE_H

#include "base_types.h"

void convert1bppFramebufferTo8bppCrtEffectFramebuffer(const dl_u8* originalImage,
                                                      dl_u8* destinationImage,
                                                      dl_u16 width,
                                                      dl_u16 height);

#endif