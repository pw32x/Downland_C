#ifndef BACKGROUND_INCLUDE_H
#define BACKGROUND_INCLUDE_H

#include "background_types.h"
#include "resource_types.h"

void Background_Draw(const BackgroundDrawData* backgroundDrawData, 
					 const Resources* resources,
					 byte* framebuffer);

#endif 
