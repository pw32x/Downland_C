#include "dl_rand.h"
#include <stdlib.h>
#include "alloc.h"

dl_u32 rng_state = 1;

void dl_srand(dl_u32 seed)
{
    rng_state = seed;
}

dl_u8 dl_rand(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
    return (rng_state & 0xff);
}
