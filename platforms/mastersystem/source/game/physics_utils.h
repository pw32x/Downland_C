#ifndef PHYSICS_UTILS_INCLUDE_H
#define PHYSICS_UTILS_INCLUDE_H

#include "base_types.h"

#define TOUCHES_VINE(v) (v & 0x10)
#define TOUCHES_TERRAIN(v) (v & 0x1)

dl_u8 testTerrainCollision(dl_u16 x, 
						dl_u16 y, 
						dl_u16 yOffset, 
						const dl_u16* objectCollisionMasks);



extern dl_u8 leftPixelData;
extern dl_u8 rightPixelData;

void getTerrainValue(dl_u16 x, 
				     dl_u16 y, 
				     dl_u16 yOffset, 
				     const dl_u16* objectCollisionMasks);

#endif