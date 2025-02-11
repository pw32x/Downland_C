#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void Game_Init(GameData* gameData, Resources* resources);
void Game_Update(GameData* gameData);
void Game_Shutdown(GameData* gameData);

#endif