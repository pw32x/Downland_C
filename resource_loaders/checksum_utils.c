#include "checksum_utils.h"

#include "..\game\base_defines.h"

static u32 swap_endian_32(u32 val) 
{
	return ((val >> 24) & 0xFF) | 
			((val >> 8) & 0xFF00) | 
			((val << 8) & 0xFF0000) | 
			((val << 24) & 0xFF000000);
}

// Verifies the checksum for Downland V1.1
BOOL checksumCheckBigEndian(const u8* fileBuffer, u32 fileBufferSize) 
{
	u32 accumulator = 0;
	u32 value = 0;
	const u32* fileBufferRunner = (const u32*)fileBuffer;

	int loopCount = fileBufferSize / sizeof(value);
	for (int loop = 0; loop < loopCount; loop++)
	{
		value = swap_endian_32(*fileBufferRunner);
		accumulator += value;
		fileBufferRunner++;
	}
		
	return (accumulator == 0x84883253);
}

// Verifies the checksum for Downland V1.1
BOOL checksumCheckLitteEndian(const u8* fileBuffer, u32 fileBufferSize) 
{
	u32 accumulator = 0;
	u32 value = 0;
	const u32* fileBufferRunner = (const u32*)fileBuffer;

	int loopCount = fileBufferSize / sizeof(value);
	for (int loop = 0; loop < loopCount; loop++)
	{
		value = *fileBufferRunner;
		accumulator += value;
		fileBufferRunner++;
	}
		
	return (accumulator == 0x84883253);
}
