#ifndef GAME_RUNNER_INCLUDE_H
#define GAME_RUNNER_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void GameRunner_Init(struct GameData* gameData, const Resources* resources);
void GameRunner_Update(struct GameData* gameData, const Resources* resources);
void GameRunner_Draw(struct GameData* gameData, const Resources* resources);

#endif