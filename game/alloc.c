#include "alloc.h"

u8 memory[6000];
u8* memoryBegin = NULL;
u8* memoryEnd = NULL;

void dl_init()
{
	memoryBegin = memory;
	memoryEnd = memory;
}

void* dl_alloc(u32 size)
{
	u8* memory = memoryEnd;

	memoryEnd += size;

	return (void*)memory;
}