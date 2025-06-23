#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void Game_Init(struct GameData* gameData, const Resources* resources);
void Game_InitPlayers(struct GameData* gameData, const Resources* resources);
void Game_Update(struct GameData* gameData, const Resources* resources);
void Game_EnterRoom(struct GameData* gameData, u8 roomNumber, const Resources* resources);

void Game_TransitionToRoom(struct GameData* gameData, u8 roomNumber, const Resources* resources);
void Game_WipeTransitionToRoom(struct GameData* gameData, u8 roomNumber, const Resources* resources);

void Game_Shutdown(struct GameData* gameData);

typedef void (*Game_ChangedRoomCallbackType)(const struct GameData* gameData, u8 roomNumber, s8 transitionType);


extern Game_ChangedRoomCallbackType Game_ChangedRoomCallback;
extern Game_ChangedRoomCallbackType Game_TransitionDone;

#endif