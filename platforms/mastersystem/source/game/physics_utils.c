#include "physics_utils.h"
#include "base_defines.h"
#include "game_data.h"

const dl_u8 collisionCheckXOffsets[4] = { 0, 0, 0, 1 };

// converted from TerrainCollisionTest in the disassembly
// no idea exactly how it works, so just mimick it.
dl_u8 terrainTest(dl_u8 pixelData) 
{
    dl_u8 floorMask = 0x55;		 // 01010101b
    dl_u8 invertedFloorMask = 0xAA; // 10101010b

    // test against the floor mask, then shift left.
    dl_u8 floorTestValue = (pixelData & floorMask) << 1;

    // test against the inverted floor, then shift right
    dl_u8 invertedFloorTestValue = (pixelData & invertedFloorMask) >> 1;

    // do the vine check?
    dl_u8 vineTestValueMaybe = (invertedFloorTestValue | floorTestValue) ^ pixelData;

	// Nonzero means touching floor, zero means not touching
	// A nonzero vineTestValueMaybe means touching vine
	// bit 0: terrain test result
	// bit 1: vine test result

	dl_u8 result = 0;

	if (~vineTestValueMaybe & pixelData)
	{
		result |= 0x10;
	}

	if ((vineTestValueMaybe & pixelData) & floorMask)
		result |= 0x1;

    return result;
}

dl_u8 testTerrainCollision(dl_u16 x, 
						dl_u16 y, 
						dl_u16 yOffset, 
						const dl_u16* objectCollisionMasks)
{
	dl_u8 pixelX = GET_HIGH_BYTE(x);
	dl_u8 tableIndex = pixelX & 0x3;
	dl_u8 collectionCheckXOffset = collisionCheckXOffsets[tableIndex]; // offset the x byte position depending on x pixel position
	dl_u16 objectCollisionMask = objectCollisionMasks[tableIndex]; // different masks for different x pixel positions

	dl_u8 sensorX = pixelX + collectionCheckXOffset;
	dl_u8 sensorY = GET_HIGH_BYTE(y) + yOffset;

	dl_u16 framebufferPosition = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

	dl_u8 firstResult = terrainTest(gameData_cleanBackground[framebufferPosition] & GET_HIGH_BYTE(objectCollisionMask));
	dl_u8 secondResult = terrainTest(gameData_cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(objectCollisionMask));

	return firstResult | secondResult;
}

dl_u8 leftPixelData;
dl_u8 rightPixelData;

void getTerrainValue(dl_u16 x, 
				     dl_u16 y, 
				     dl_u16 yOffset, 
				     const dl_u16* objectCollisionMasks)
{
	dl_u8 pixelX = GET_HIGH_BYTE(x);
	dl_u8 tableIndex = pixelX & 0x3;
	dl_u8 collectionCheckXOffset = collisionCheckXOffsets[tableIndex]; // offset the x byte position depending on x pixel position
	dl_u16 objectCollisionMask = objectCollisionMasks[tableIndex]; // different masks for different x pixel positions

	dl_u8 sensorX = pixelX + collectionCheckXOffset;
	dl_u8 sensorY = GET_HIGH_BYTE(y) + yOffset;

	dl_u16 framebufferPosition = GET_FRAMEBUFFER_LOCATION(sensorX, sensorY);

	leftPixelData = gameData_cleanBackground[framebufferPosition] & GET_HIGH_BYTE(objectCollisionMask);
	rightPixelData = gameData_cleanBackground[framebufferPosition + 1] & GET_LOW_BYTE(objectCollisionMask);

	if (leftPixelData || rightPixelData)
		terrainTest(1);
}