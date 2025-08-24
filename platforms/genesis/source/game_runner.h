#ifndef GAME_RUNNER_INCLUDE_H
#define GAME_RUNNER_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void GameRunner_Init(GameData* gameData, const Resources* resources);
void GameRunner_Update(GameData* gameData, const Resources* resources);
void GameRunner_Draw(GameData* gameData, const Resources* resources);

#endif