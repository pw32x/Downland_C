#ifndef PLAYER_TYPES_INCLUDE_H
#define PLAYER_TYPES_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"
#include "joystick_types.h"
#include "door_types.h"
#include "string_utils.h"
#include "rooms/rooms.h"

#define PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE (PLAYER_SPRITE_ROWS * 3) // rows * 3 bytes per row

#define PLAYER_SPLAT_SPRITE_WIDTH			24
#define PLAYER_SPLAT_SPRITE_FRAME_SIZE	(PLAYER_SPLAT_SPRITE_ROWS * 3)

#define PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE (PLAYER_COLLISION_MASK_ROWS * 3) // rows * 3 bytes per row

#define PLAYERONE_BITMASK 0x1
#define PLAYERTWO_BITMASK 0x2

#define ROOM_TIMER_DEFAULT 4096
#define ROOM_TIMER_HALF_TIME 2048

typedef struct
{
	dl_u8 state;	
	dl_u8 lives;
	dl_u8 gameOver;
	dl_u16 x; // high resolution position 256 pixels, 256 subpixels
	dl_u16 y; // high resolution position 256 pixels, 256 subpixels
	dl_u16 speedx;
	dl_u16 speedy;
	dl_u8 playerNumber;
	dl_u8 playerMask;
	dl_u8 currentFrameNumber; // 0 to 3 for run animation (both directions), 4 to 5 for climbing animation
	dl_u8 currentSpriteNumber; // 0 to 3 run right, 4/5 climb, 6-9 run left
	const dl_u8* currentSprite;
	const dl_u8* bitShiftedSprites;
	const dl_u8* bitShiftedCollisionMasks;
	const dl_u8* bitShiftedSplatSprite;

	dl_u8 facingDirection;

	dl_u8 jumpAirCounter;
	dl_u8 ignoreRopesCounter;

	dl_u8 holdLeftCounter;
	dl_u8 holdRightCounter;

	dl_u8 preserveAirMomentum; // TRUE when jumping but FALSE when falling past
							// max falling speed. FALSE when walking off ledges.

	dl_u32 score;
	dl_u8 scoreString[SCORE_STRING_SIZE];

	dl_u8 cantMoveCounter;
	dl_u16 regenerationCounter;
	dl_u8 isDead;

	dl_u8 globalAnimationCounter; // drives running, climbing animation

	
	RoomPickups gamePickups;
	dl_u8 doorStateData[DOOR_TOTAL_COUNT];
	dl_u8 gameCompletionCount;
	Room* currentRoom;

	dl_u16 roomTimers[NUM_ROOMS];

	const DoorInfo* lastDoor;

	dl_u8 splatFrameNumber;
} PlayerData;

#endif