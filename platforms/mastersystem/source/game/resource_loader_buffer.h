#ifndef RESOURCE_LOADER_BUFFER_INCLUDE_H
#define RESOURCE_LOADER_BUFFER_INCLUDE_H

#ifndef DISABLE_RESOURCE_LOADER

#include "../game/base_types.h"
#include "../game/resource_types.h"

dl_u8 ResourceLoaderBuffer_Init(const dl_u8* fileBuffer, dl_u32 fileBufferSize, Resources* resources);
void ResourceLoaderBuffer_Shutdown(Resources* resources);

#endif

#endif