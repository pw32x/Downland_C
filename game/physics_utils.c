#include "physics_utils.h"
#include "base_defines.h"

u8 collisionCheckXOffsets[4] = { 0, 0, 0, 1 };

// converted from TerrainCollisionTest in the disassembly
// no idea exactly how it works, so just mimick it.
u8 terrainTest(u8 pixelData) 
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
	// bit 0: terrain test result
	// bit 1: vine test result

	u8 result = 0;

	if (~vineTestValueMaybe & pixelData)
	{
		result |= 0x10;
	}

	if ((vineTestValueMaybe & pixelData) & floorMask)
		result |= 0x1;

    return result;
}

u8 testTerrainCollision(u16 x, 
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

	u8 firstResult = terrainTest(cleanBackground[framebufferPosition] & GET_HIGH_BYTE(objectCollisionMask));
	u8 secondResult = terrainTest(cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(objectCollisionMask));

	return firstResult | secondResult;
}

u8 leftPixelData;
u8 rightPixelData;

void getTerrainValue(u16 x, 
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

	leftPixelData = cleanBackground[framebufferPosition] & GET_HIGH_BYTE(objectCollisionMask);
	rightPixelData = cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(objectCollisionMask);

	if (leftPixelData || rightPixelData)
		terrainTest(1);
}