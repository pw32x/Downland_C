#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void Game_Init(struct GameData* gameData, Resources* resources);
void Game_Update(struct GameData* gameData, Resources* resources);
void Game_EnterRoom(struct GameData* gameData, u8 roomNumber, Resources* resources);
void Game_TransitionToRoom(struct GameData* gameData, u8 roomNumber, Resources* resources);
void Game_WipeTransitionToRoom(struct GameData* gameData, u8 roomNumber, Resources* resources);
void Game_Shutdown(struct GameData* gameData);

#endif