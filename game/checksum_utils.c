#include "checksum_utils.h"

#include "..\game\base_defines.h"

static dl_u32 swap_endian_32(dl_u32 val) 
{
	return ((val >> 24) & 0xFF) | 
			((val >> 8) & 0xFF00) | 
			((val << 8) & 0xFF0000) | 
			((val << 24) & 0xFF000000);
}

// Verifies the checksum for Downland V1.1
BOOL checksumCheckBigEndian(const dl_u8* fileBuffer, dl_u32 fileBufferSize) 
{
	dl_u32 accumulator = 0;
	dl_u32 value = 0;
	const dl_u32* fileBufferRunner = (const dl_u32*)fileBuffer;

	dl_u32 loopCount = fileBufferSize / sizeof(value);

	dl_u32 loop;
	for (loop = 0; loop < loopCount; loop++)
	{
		value = swap_endian_32(*fileBufferRunner);
		accumulator += value;
		fileBufferRunner++;
	}
		
	return (accumulator == 0x84883253);
}

// Verifies the checksum for Downland V1.1
BOOL checksumCheckLitteEndian(const dl_u8* fileBuffer, dl_u32 fileBufferSize) 
{
	dl_u32 accumulator = 0;
	dl_u32 value = 0;
	const dl_u32* fileBufferRunner = (const dl_u32*)fileBuffer;

	dl_u32 loopCount = fileBufferSize / sizeof(value);

	dl_u32 loop;
	for (loop = 0; loop < loopCount; loop++)
	{
		value = *fileBufferRunner;
		accumulator += value;
		fileBufferRunner++;
	}
		
	return (accumulator == 0x84883253);
}
