#include "alloc.h"

static u8 memory[6000];
static u8* memoryBegin = NULL;
static u8* memoryEnd = NULL;

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