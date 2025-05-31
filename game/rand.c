#include "rand.h"
#include <stdlib.h>
#include "alloc.h"

u8* randRunner = NULL;

// very crappy pseudorandom number generator
// using the memory buffer as the source of
// random numbers.
u8 dl_rand()
{
	if (randRunner < memoryEnd - 1)
		return *++randRunner;

	randRunner = memoryBegin;
	return *randRunner;
}

void dl_srand(u32 seed)
{
	//							 123456 % 6000
	randRunner = memoryBegin + (seed % (memoryEnd - memoryBegin));
}