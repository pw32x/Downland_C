#include "draw_utils.h"

// depending on the x position, chose 
// of these to get at the corresponding
// two bits you want to activate.
dl_u8 pixelMasks[4] = 
{
	0xc0, // 11000000b,
	0x30, // 00110000b,
	0x0C, // 00001100b,
	0x03  // 00000011b,
};
