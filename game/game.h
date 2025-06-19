#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void Game_Init(struct GameData* gameData, Resources* resources);
void Game_InitPlayers(struct GameData* gameData, Resources* resources);
void Game_Update(struct GameData* gameData, Resources* resources);
void Game_EnterRoom(struct GameData* gameData, u8 roomNumber, Resources* resources);


// Define OVERRIDE_TRANSITIONS to override the default transition
// behavior in case the client platform wants to handle the transition 
// effect itself. Do this for platforms that can't do the per-pixel wiping fast 
// enough or at all and want to do something more viable on their hardware.
void Game_TransitionToRoom(struct GameData* gameData, u8 roomNumber, Resources* resources);
void Game_WipeTransitionToRoom(struct GameData* gameData, u8 roomNumber, Resources* resources);

void Game_Shutdown(struct GameData* gameData);

typedef void (*Game_ChangedRoomCallbackType)(const struct GameData* gameData, u8 roomNumber, s8 transitionType);

extern Game_ChangedRoomCallbackType Game_ChangedRoomCallback;

#endif