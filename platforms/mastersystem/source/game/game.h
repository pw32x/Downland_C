#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "resource_types.h"

void Game_Init(const Resources* resources, dl_u8* cleanBackground);
void Game_InitPlayers(const Resources* resources);
void Game_Update(const Resources* resources);
void Game_EnterRoom(dl_u8 roomNumber, const Resources* resources);

void Game_TransitionToRoom(dl_u8 roomNumber, const Resources* resources);
void Game_WipeTransitionToRoom(dl_u8 roomNumber, const Resources* resources);

typedef void (*Game_ChangedRoomCallbackType)(dl_u8 roomNumber, dl_s8 transitionType);

extern Game_ChangedRoomCallbackType Game_ChangedRoomCallback;
extern Game_ChangedRoomCallbackType Game_TransitionDone;

#endif