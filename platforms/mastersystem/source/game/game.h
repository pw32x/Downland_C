#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "resource_types.h"

void Game_Init(void);
void Game_InitPlayers(void);
void Game_Update(void);
void Game_EnterRoom(dl_u8 roomNumber);

void Game_TransitionToRoom(dl_u8 roomNumber);
void Game_WipeTransitionToRoom(dl_u8 roomNumber);

typedef void (*Game_ChangedRoomCallbackType)(dl_u8 roomNumber, dl_s8 transitionType);

extern Game_ChangedRoomCallbackType Game_ChangedRoomCallback;

#endif