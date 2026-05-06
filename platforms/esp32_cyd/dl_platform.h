#ifndef DL_PLATFORM_INCLUDE_H
#define DL_PLATFORM_INCLUDE_H

#include "base_types.h"

void dl_memset(void* source, dl_u8 value, dl_u16 count);
void dl_memcpy(void* destination, const void* source, dl_u16 count);

#endif