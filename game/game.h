#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"
#include "../resources/resources.h"
#include "drops_manager.h"

typedef struct
{
	u8 framebuffer[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // main game 1bpp frame buffer
	u8 cleanBackground[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // the game background without UI or objects. Used for terrain collision detection.
	u32 crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
	DropData dropData;
	u8 gameCompletionCount;
	u8 roomNumber;
} GameData;

void Game_Init(GameData* gameData, Resources* resources);
void Game_Update(GameData* gameData);
void Game_Shutdown(GameData* gameData);

#endif