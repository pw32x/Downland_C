#ifndef DRAW_BACKGROUND_INCLUDE_H
#define DRAW_BACKGROUND_INCLUDE_H

#include "../resources/resources.h"

void Draw_Background(const BackgroundDrawData* backgroundDrawData, 
					 const Resources* resources,
					 byte* framebuffer);

#endif 
