#ifndef ALLOC_INCLUDE_H
#define ALLOC_INCLUDE_H

#include "base_types.h"

// use a fixed array for memory. pretty much used
// for loading resources at start up.
// dl_rand() uses the memory as its source of random
// numbers, similar to how the original game would
// cycle through the rom byte by byte to provide random
// values.
extern u8* memoryBegin;
extern u8* memoryEnd;

void dl_init();
void* dl_alloc(u32 size);

#endif