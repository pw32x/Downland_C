#ifndef PLAYER_INCLUDE_H
#define PLAYER_INCLUDE_H

#include "base_types.h"
#include "resource_types.h"
#include "joystick_types.h"

#define PLAYER_SPRITE_COUNT			10
#define PLAYER_SPRITE_ROWS			16
#define PLAYER_BITSHIFTED_SPRITE_FRAME_SIZE (PLAYER_SPRITE_ROWS * 3) // rows * 3 bytes per row

typedef struct
{
	u8 enabled;
	u8 state;	
	u16 x; // high resolution position 256 pixels, 256 subpixels
	u16 y; // high resolution position 256 pixels, 256 subpixels
	u16 speedx;
	u16 speedy;
	u8 currentFrame;
	u8* currentSprite;
	u8* bitShiftedSprites;

	u8 facingDirection;

	u8 isClimbing;
	u8 isJumping;
} PlayerData;

void Player_Init(PlayerData* playerData, const Resources* resources);
void Player_Update(PlayerData* playerData, const JoystickState* joystickState, u8* framebuffer, u8* cleanBackground);

#endif