#ifndef BACKGROUND_INCLUDE_H
#define BACKGROUND_INCLUDE_H

#include "background_types.h"
#include "../resources/resources.h"

void Background_Draw(const BackgroundDrawData* backgroundDrawData, 
					 const Resources* resources,
					 byte* framebuffer);

#endif 
