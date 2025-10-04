#include "dl_rand.h"

static dl_u16 rng16 = 0xACE1;  // seed

void dl_srand(dl_u32 seed)
{
    rng16 = seed;
}

dl_u8 dl_rand(void)
{
    rng16 = (rng16 * 25173u) + 13849u;  // proven full-period constants
    return (dl_u8)(rng16 >> 8);         // use the high byte (better distributed)
}