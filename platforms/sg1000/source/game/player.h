#ifndef PLAYER_INCLUDE_H
#define PLAYER_INCLUDE_H

#include "player_types.h"
#include "resource_types.h"


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

#define PLAYER_START_X 0x70 // 112
#define PLAYER_START_Y 0xa5 // 165

void Player_GameInit(PlayerData* playerData);
void Player_RoomInit(PlayerData* playerData);

void Player_Update(PlayerData* playerData, 
				   const DoorInfoData* doorInfoData,
				   dl_u8* doorStateData);

void Player_StartRegen(PlayerData* playerData);

void Player_PerformCollisions(void);

void Player_CompleteGameLoop(PlayerData* playerData);
#endif