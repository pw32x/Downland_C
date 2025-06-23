#ifndef RESOURCE_LOADER_BUFFER_INCLUDE_H
#define RESOURCE_LOADER_BUFFER_INCLUDE_H

#include "..\game\base_types.h"
#include "..\game\resource_types.h"

int ResourceLoaderBuffer_Init(const u8* fileBuffer, u32 fileBufferSize, Resources* resources);
void ResourceLoaderBuffer_Shutdown(Resources* resources);

#endif