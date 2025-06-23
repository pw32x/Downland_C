#ifndef RESOURCE_LOADER_BUFFER_INCLUDE_H
#define RESOURCE_LOADER_BUFFER_INCLUDE_H

#include "..\game\base_types.h"
#include "..\game\resource_types.h"

#define RESLOAD_BUFFER_RESULT_OK				0
#define RESLOAD_BUFFER_RESULT_CHECKSUMFAILED	1
#define RESLOAD_BUFFER_RESULT_WRONGROMSIZE		2
#define RESLOAD_BUFFER_RESULT_UNKNOWNFAILURE	0xff

int ResourceLoaderBuffer_Init(const u8* fileBuffer, u32 fileBufferSize, Resources* resources);
void ResourceLoaderBuffer_Shutdown(Resources* resources);

#endif