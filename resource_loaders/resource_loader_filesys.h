#ifndef RESOURCE_LOADER_FILESYS_INCLUDE_H
#define RESOURCE_LOADER_FILESYS_INCLUDE_H

#include "..\game\base_types.h"
#include "..\game\resource_types.h"

BOOL ResourceLoaderFileSys_Init(const char* romPath, Resources* resources);
void ResourceLoaderFileSys_Shutdown(Resources* resources);



#endif