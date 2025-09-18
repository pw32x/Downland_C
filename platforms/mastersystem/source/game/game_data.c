#include "game_data.h"

dl_u8* gameData_cleanBackground; // the game background without UI or objects. Used for terrain collision detection.

// objects
PlayerData gameData_playerData[NUM_PLAYERS];

// there's no distinct player one or player two
// the pointers swap every time a player dies.
PlayerData* gameData_currentPlayerData;
PlayerData* gameData_otherPlayerData;
dl_u8 gameData_numPlayers;

const Room* gameData_currentRoom;

// used for screen transitions
dl_u8 gameData_transitionRoomNumber;
dl_u16 gameData_transitionInitialDelay;
dl_u8 gameData_transitionCurrentLine;
dl_u8 gameData_transitionFrameDelay;

// strings. Number of expected characters + 0xff ending value
dl_u8 gameData_string_roomNumber[ROOM_NUMBER_STRING_SIZE];
dl_u8 gameData_string_timer[TIMER_STRING_SIZE]; // max timer is 65535
dl_u8 gameData_string_highScore[SCORE_STRING_SIZE];

dl_u32 gameData_highScore;
dl_u8 gameData_paused;