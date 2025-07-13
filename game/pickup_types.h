#ifndef PICKUP_TYPES_INCLUDE_H
#define PICKUP_TYPES_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

#define NUM_PICKUPS_PER_ROOM 5

#define PICKUP_HEIGHT	0xa
#define PICKUP_WIDTH	0x8

#define PICKUPS_NUM_SPRITE_WIDTH 16
#define PICKUPS_NUM_SPRITE_ROWS 10

#define PICKUP_KEY_POINTS		0xc8  // 200
#define PICKUP_MONEYBAG_POINTS	0x12c // 300 
#define PICKUP_DIAMOND_POINTS	0x190 // 400

// pick up positions stored in rom
typedef PACKED struct
{
	dl_u8 y;
	dl_u8 x;
} PickupPosition;

// runtime pickup data
#define INIT_PICKUP_STATE 0x3 // both bits are on, meaning that they're available
						      // for both players

#define PICKUP_TYPE_DIAMOND		0
#define PICKUP_TYPE_MONEYBAG	1
#define PICKUP_TYPE_KEY			2

typedef struct
{
	dl_u8 state;	// keystate contains state for both players
	dl_u8 type;
	dl_u8 x;
	dl_u8 y;
	dl_u8 doorUnlockIndex;
} Pickup;

// in each set of five pick ups per room, the first two
// are assumed to be keys. The three others, randomly 
// picked treasures of diamonds and money bags.
typedef Pickup RoomPickups[NUM_ROOMS][NUM_PICKUPS_PER_ROOM];

#endif