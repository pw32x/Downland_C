#ifndef PLAYER_INCLUDE_H
#define PLAYER_INCLUDE_H

#include "base_types.h"
#include "player_types.h"
#include "resource_types.h"
#include "joystick_types.h"
#include "door_types.h"
#include "string_utils.h"
#include "rooms.h"
#include "game.h"

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