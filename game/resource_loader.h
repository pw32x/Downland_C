#ifndef RESOURCE_LOADER_INCLUDE_H
#define RESOURCE_LOADER_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"

BOOL ResourceLoader_Init(const char* romPath, Resources* resources);
void ResourceLoader_Shutdown(Resources* resources);

#endif