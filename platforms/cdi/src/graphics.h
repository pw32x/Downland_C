#ifndef __GRAPHICS_H__
#define	__GRAPHICS_H__

#include "video.h"

#define SIG_BLANK 0x0100

#define BUFFER_START (drawVideoBuffer + pixelStart)

extern u_char* drawVideoBuffer;
extern unsigned int frameDone;

#endif