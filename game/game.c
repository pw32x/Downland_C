#include "game.h"

#include "draw_utils.h"
#include "rooms\rooms.h"
#include "alloc.h"

Game_ChangedRoomCallbackType Game_ChangedRoomCallback = NULL;
Game_ChangedRoomCallbackType Game_TransitionDone = NULL;

static dl_u8 initializedFramebuffers = FALSE;

void Game_Init(struct GameData* gameData, const Resources* resources)
{
	if (!initializedFramebuffers)
	{
		initializedFramebuffers = TRUE;
		gameData->framebuffer = (dl_u8*)dl_alloc(FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH);
		gameData->cleanBackground = (dl_u8*)dl_alloc(FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH);
	}

	gameData->numPlayers = 1;

	// init strings
	gameData->string_roomNumber[ROOM_NUMBER_STRING_SIZE - 1] = 0xff; // end of line
	gameData->string_timer[TIMER_STRING_SIZE - 1] = 0xff;
	gameData->playerData[PLAYER_ONE].scoreString[SCORE_STRING_SIZE - 1] = 0xff;
	gameData->playerData[PLAYER_TWO].scoreString[SCORE_STRING_SIZE - 1] = 0xff;
	gameData->string_highScore[SCORE_STRING_SIZE - 1] = 0xff;

	gameData->highScore = 0;

	gameData->playerData[PLAYER_ONE].playerNumber = PLAYER_ONE;
	gameData->playerData[PLAYER_ONE].playerMask = PLAYERONE_BITMASK;

	gameData->playerData[PLAYER_TWO].playerNumber = PLAYER_TWO;
	gameData->playerData[PLAYER_TWO].playerMask = PLAYERTWO_BITMASK;

#ifdef START_AT_TITLESCREEN
	// init title screen
	Game_TransitionToRoom(gameData, TITLESCREEN_ROOM_INDEX, resources);
#else
	gameData->currentPlayerData = &gameData->playerData[PLAYER_ONE];
	gameData->otherPlayerData = NULL;
	Player_GameInit(gameData->currentPlayerData, resources);
	Game_TransitionToRoom(gameData, 0 /*roomNumber*/, resources);
#endif
}

void Game_InitPlayers(struct GameData* gameData, const Resources* resources)
{
	gameData->currentPlayerData = &gameData->playerData[PLAYER_ONE];

	gameData->otherPlayerData = gameData->numPlayers > 1 ? &gameData->playerData[PLAYER_TWO] : NULL;

	for (dl_u8 loop = 0; loop < gameData->numPlayers; loop++)
	{
		Player_GameInit(&gameData->playerData[loop], resources);
	}
}

void Game_Update(struct GameData* gameData, const Resources* resources)
{
	gameData->currentRoom->update((struct Room*)gameData->currentRoom, 
								  (struct GameData*)gameData, 
								  resources);
}

void Game_EnterRoom(struct GameData* gameData, dl_u8 roomNumber, const Resources* resources)
{
	gameData->currentRoom = g_rooms[roomNumber];

	if (gameData->currentPlayerData != NULL)
		gameData->currentPlayerData->currentRoom = gameData->currentRoom;

	gameData->currentRoom->init(gameData->currentRoom, 
								(struct GameData*)gameData, 
								resources);

	if (Game_ChangedRoomCallback != NULL)
		Game_ChangedRoomCallback(gameData, roomNumber, -1);
}

void Game_TransitionToRoom(struct GameData* gameData, dl_u8 roomNumber, const Resources* resources)
{
	gameData->transitionRoomNumber = roomNumber;

	gameData->currentRoom = g_rooms[roomNumber];

	if (gameData->currentPlayerData != NULL)
		gameData->currentPlayerData->currentRoom = gameData->currentRoom;

	gameData->currentRoom = g_rooms[TRANSITION_ROOM_INDEX];
	gameData->currentRoom->init(g_rooms[roomNumber], 
								(struct GameData*)gameData, 
								resources);

	if (Game_ChangedRoomCallback != NULL)
		Game_ChangedRoomCallback(gameData, roomNumber, TRANSITION_ROOM_INDEX);
}

void Game_WipeTransitionToRoom(struct GameData* gameData, dl_u8 roomNumber, const Resources* resources)
{
	gameData->transitionRoomNumber = roomNumber;

	gameData->currentRoom = g_rooms[roomNumber];

	if (gameData->currentPlayerData != NULL)
		gameData->currentPlayerData->currentRoom = gameData->currentRoom;

	gameData->currentRoom = g_rooms[WIPE_TRANSITION_ROOM_INDEX];
	gameData->currentRoom->init(g_rooms[roomNumber], 
								(struct GameData*)gameData, 
								resources);

	if (Game_ChangedRoomCallback != NULL)
		Game_ChangedRoomCallback(gameData, roomNumber, WIPE_TRANSITION_ROOM_INDEX);
}

void Game_Shutdown(struct GameData* gameData)
{

}
