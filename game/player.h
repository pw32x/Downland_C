#ifndef PLAYER_INCLUDE_H
#define PLAYER_INCLUDE_H

#include "base_types.h"
#include "player_types.h"
#include "resource_types.h"
#include "joystick_types.h"
#include "door_types.h"
#include "string_utils.h"
#include "rooms\rooms.h"
#include "game.h"

// all the states are mutually exclusive
#define PLAYER_STATE_STAND			0
#define PLAYER_STATE_RUN			1
#define PLAYER_STATE_JUMP			2
#define PLAYER_STATE_FALL			3
#define PLAYER_STATE_CLIMB			4
#define PLAYER_STATE_HANG_LEFT		5
#define PLAYER_STATE_HANG_RIGHT		6
#define PLAYER_STATE_REGENERATION	7
#define PLAYER_STATE_SPLAT			8
#define PLAYER_MIDAIR_DEATH			9
#define PLAYER_STATE_DEBUG			0xff

void Player_GameInit(PlayerData* playerData, const Resources* resources);
void Player_RoomInit(PlayerData* playerData, const Resources* resources);

void Player_Update(PlayerData* playerData, 
				   const JoystickState* joystickState, 
				   u8* framebuffer, 
				   u8* cleanBackground, 
				   DoorInfoData* doorInfoData,
				   u8* doorStateData);

void Player_StartRegen(PlayerData* playerData);

u8 Player_HasCollision(PlayerData* playerData, u8* framebuffer, u8* cleanBackground);
void Player_PerformCollisions(struct GameData* gameData, Resources* resources);

void Player_CompleteGameLoop(PlayerData* playerData, const Resources* resource);
#endif