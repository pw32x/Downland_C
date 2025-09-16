#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "game_types.h"
#include "resource_types.h"

void Game_Init(struct GameData* gameData, const Resources* resources, dl_u8* cleanBackground);
void Game_InitPlayers(struct GameData* gameData, const Resources* resources);
void Game_Update(struct GameData* gameData, const Resources* resources);
void Game_EnterRoom(struct GameData* gameData, dl_u8 roomNumber, const Resources* resources);

void Game_TransitionToRoom(struct GameData* gameData, dl_u8 roomNumber, const Resources* resources);
void Game_WipeTransitionToRoom(struct GameData* gameData, dl_u8 roomNumber, const Resources* resources);

typedef void (*Game_ChangedRoomCallbackType)(const struct GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);


extern Game_ChangedRoomCallbackType Game_ChangedRoomCallback;
extern Game_ChangedRoomCallbackType Game_TransitionDone;

#endif