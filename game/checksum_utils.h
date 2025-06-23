#ifndef CHECKSUM_UTILS_INCLUDE_H
#define CHECKSUM_UTILS_INCLUDE_H

#include "..\game\base_types.h"
#include "..\game\resource_types.h"

BOOL checksumCheckLitteEndian(const u8* fileBuffer, u32 fileBufferSize);
BOOL checksumCheckBigEndian(const u8* fileBuffer, u32 fileBufferSize);

#endif