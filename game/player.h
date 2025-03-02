#ifndef PLAYER_INCLUDE_H
#define PLAYER_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"
#include "joystick_types.h"
#include "door_types.h"

#define PLAYER_SPRITE_COUNT			10
#define PLAYER_SPRITE_ROWS			16
#define PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE (PLAYER_SPRITE_ROWS * 3) // rows * 3 bytes per row

#define PLAYER_COLLISION_WIDTH		8

#define PLAYER_COLLISION_MASK_ROWS	5
#define PLAYER_BITSHIFTED_COLLISION_MASK_FRAME_SIZE (PLAYER_COLLISION_MASK_ROWS * 3) // rows * 3 bytes per row

#define PLAYERONE_MASK 0x1
#define PLAYERTWO_MASK 0x2

struct GameData;

typedef struct
{
	u8 state;	
	u16 x; // high resolution position 256 pixels, 256 subpixels
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u16 speedx;
	u16 speedy;
	u8 playerMask;
	u8 currentFrameNumber; // 0 to 3 for run animation (both directions), 4 to 5 for climbing animation
	u8 currentSpriteNumber; // 0 to 3 run right, 4/5 climb, 6-9 run left
	u8* currentSprite;
	u8* bitShiftedSprites;
	u8* bitShiftedCollisionMasks;

	u8 facingDirection;

	u8 jumpAirCounter;
	u8 ignoreRopesCounter;

	u8 safeLanding;

	u32* score; // points to score stored in gameData
	u8* scoreString;

	u8 cantMoveCounter;
	u16 regenerationCounter;

	u8 globalAnimationCounter; // drives running, climbing animation

	DoorInfo* lastDoor;
} PlayerData;

void Player_GameInit(PlayerData* playerData, const Resources* resources);
void Player_RoomInit(PlayerData* playerData, const Resources* resources);

void Player_Update(PlayerData* playerData, 
				   const JoystickState* joystickState, 
				   u8* framebuffer, 
				   u8* cleanBackground, 
				   DoorInfoData* doorInfoData,
				   u8* doorStateData);

u8 Player_HasCollision(PlayerData* playerData, u8* framebuffer, u8* cleanBackground);
void Player_PerformCollisions(struct GameData* gameData, Resources* resources);
#endif