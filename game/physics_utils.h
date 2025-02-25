#ifndef PHYSICS_UTILS_INCLUDE_H
#define PHYSICS_UTILS_INCLUDE_H

#include "base_types.h"

#define TOUCHES_VINE(v) (v & 0x10)
#define TOUCHES_TERRAIN(v) (v & 0x1)

u8 testTerrainCollision(u16 x, u16 y, u16 yOffset, u16* objectCollisionMasks, u8* cleanBackground);



extern u8 leftPixelData;
extern u8 rightPixelData;

void getTerrainValue(u16 x, 
				     u16 y, 
				     u16 yOffset, 
				     u16* objectCollisionMasks,
				     u8* cleanBackground);

#endif