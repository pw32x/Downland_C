#ifndef PLAYER_TYPES_INCLUDE_H
#define PLAYER_TYPES_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"
#include "joystick_types.h"
#include "door_types.h"
#include "string_utils.h"
#include "rooms\rooms.h"

#define PLAYER_SPRITE_WIDTH			16
#define PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE (PLAYER_SPRITE_ROWS * 3) // rows * 3 bytes per row

#define PLAYER_COLLISION_WIDTH		8

#define PLAYER_SPLAT_SPRITE_WIDTH			24
#define PLAYER_SPLAT_SPRITE_FRAME_SIZE	(PLAYER_SPLAT_SPRITE_ROWS * 3)

#define PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE (PLAYER_COLLISION_MASK_ROWS * 3) // rows * 3 bytes per row

#define PLAYERONE_BITMASK 0x1
#define PLAYERTWO_BITMASK 0x2

#define ROOM_TIMER_DEFAULT 4096

typedef struct
{
	u8 state;	
	u8 lives;
	u8 gameOver;
	u16 x; // high resolution position 256 pixels, 256 subpixels
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u16 speedx;
	u16 speedy;
	u8 playerNumber;
	u8 playerMask;
	u8 currentFrameNumber; // 0 to 3 for run animation (both directions), 4 to 5 for climbing animation
	u8 currentSpriteNumber; // 0 to 3 run right, 4/5 climb, 6-9 run left
	const u8* currentSprite;
	const u8* bitShiftedSprites;
	const u8* bitShiftedCollisionMasks;
	const u8* bitShiftedSplatSprite;

	u8 facingDirection;

	u8 jumpAirCounter;
	u8 ignoreRopesCounter;

	u8 holdLeftCounter;
	u8 holdRightCounter;

	u8 preserveAirMomentum; // TRUE when jumping but FALSE when falling past
							// max falling speed. FALSE when walking off ledges.

	u32 score;
	u8 scoreString[SCORE_STRING_SIZE];

	u8 cantMoveCounter;
	u16 regenerationCounter;
	u8 isDead;

	u8 globalAnimationCounter; // drives running, climbing animation

	
	RoomPickups gamePickups;
	u8 doorStateData[DOOR_TOTAL_COUNT];
	u8 gameCompletionCount;
	Room* currentRoom;

	u16 roomTimers[NUM_ROOMS];

	const DoorInfo* lastDoor;

	u8 splatFrameNumber;
} PlayerData;

#endif