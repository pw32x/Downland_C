#include "physics_utils.h"
#include "base_defines.h"

u8 collisionCheckXOffsets[4] = { 0, 0, 0, 1 };

// converted from TerrainCollisionTest in the disassembly
// no idea exactly how it works. Just mimick it.
u8 check_collision(u8 pixelData) 
{
    u8 floorMask = 0x55;		 // 01010101b
    u8 invertedFloorMask = 0xAA; // 10101010b

    // test against the floor mask, then shift left.
    u8 floorTestValue = (pixelData & floorMask) << 1;

    // test against the inverted floor, then shift right
    u8 invertedFloorTestValue = (pixelData & invertedFloorMask) >> 1;

    // do the vine check?
    u8 vineTestValueMaybe = (invertedFloorTestValue | floorTestValue) ^ pixelData;

	// Nonzero means touching floor, zero means not touching
	// A nonzero vineTestValueMaybe means touching vine
    return (vineTestValueMaybe & pixelData) & floorMask; 
}

u8 testCollision(u16 x, 
				 u16 y, 
				 u16 yOffset, 
				 u16* objectCollisionMasks,
				 u8* cleanBackground)
{
	u8 pixelX = GET_HIGH_BYTE(x);
	u8 tableIndex = pixelX & 0x3;
	u8 collectionCheckXOffset = collisionCheckXOffsets[tableIndex]; // offset the x byte position depending on x pixel position
	u16 objectCollisionMask = objectCollisionMasks[tableIndex]; // different masks for different x pixel positions

	u8 sensorX = pixelX + collectionCheckXOffset;
	u8 sensorY = GET_HIGH_BYTE(y) + yOffset;

	u16 framebufferPosition = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

	u8 firstByte = cleanBackground[framebufferPosition] & GET_HIGH_BYTE(objectCollisionMask);
	u8 secondByte = cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(objectCollisionMask);

	return check_collision(firstByte) != 0 ||
		   check_collision(secondByte) != 0;
}

