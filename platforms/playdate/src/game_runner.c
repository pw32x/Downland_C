#include "game_runner.h"

#include "..\..\..\game\drops_manager.h"
#include "..\..\..\game\draw_utils.h"
#include "..\..\..\game\rooms\chambers.h"

void GameRunner_Init(struct GameData* gameData, const Resources* resources)
{
	Game_Init(gameData, resources);
}

void GameRunner_Update(struct GameData* gameData, const Resources* resources)
{
	Game_Update(gameData, resources);
}

void GameRunner_Draw(struct GameData* gameData, const Resources* resources)
{
	//m_drawRoomFunctions[gameData->currentRoom->roomNumber](gameData, resources);
}
