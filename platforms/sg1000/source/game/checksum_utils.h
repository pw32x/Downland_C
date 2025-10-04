#ifndef CHECKSUM_UTILS_INCLUDE_H
#define CHECKSUM_UTILS_INCLUDE_H

#include "../game/base_types.h"
#include "../game/resource_types.h"

BOOL checksumCheckLitteEndian(const dl_u8* fileBuffer, dl_u32 fileBufferSize);
BOOL checksumCheckBigEndian(const dl_u8* fileBuffer, dl_u32 fileBufferSize);

#endif