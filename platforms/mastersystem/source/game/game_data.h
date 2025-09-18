#ifndef GAME_TYPES
#define GAME_TYPES

#include "base_types.h"
#include "base_defines.h"
#include "drops_types.h"
#include "rooms/rooms.h"
#include "resource_types.h"
#include "pickup_types.h"
#include "string_utils.h"
#include "ball.h"
#include "bird.h"
#include "player.h"

#define NUM_PLAYERS 2
#define PLAYER_ONE	0
#define PLAYER_TWO	1

// contains the global state of the game

extern dl_u8* gameData_cleanBackground; // the game background without UI or objects. Used for terrain collision detection.

// objects
extern PlayerData gameData_playerData[NUM_PLAYERS];

// there's no distinct player one or player two
// the pointers swap every time a player dies.
extern PlayerData* gameData_currentPlayerData;
extern PlayerData* gameData_otherPlayerData;
extern dl_u8 gameData_numPlayers;

extern const Room* gameData_currentRoom;

// used for screen transitions
extern dl_u8 gameData_transitionRoomNumber;
extern dl_u16 gameData_transitionInitialDelay;
extern dl_u8 gameData_transitionCurrentLine;
extern dl_u8 gameData_transitionFrameDelay;

// strings. Number of expected characters + 0xff ending value
extern dl_u8 gameData_string_roomNumber[ROOM_NUMBER_STRING_SIZE];
extern dl_u8 gameData_string_timer[TIMER_STRING_SIZE]; // max timer is 65535
extern dl_u8 gameData_string_highScore[SCORE_STRING_SIZE];

extern dl_u32 gameData_highScore;
extern dl_u8 gameData_paused;

#endif