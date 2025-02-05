#ifndef GAME_INCLUDE_H
#define GAME_INCLUDE_H

#include "base_types.h"
#include "base_defines.h"

typedef struct
{
	u8 framebuffer[FRAMEBUFFER_HEIGHT * FRAMEBUFFER_PITCH]; // main game 1bpp frame buffer
	u32 crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
} GameData;

void Game_Init(GameData* gameData);
void Game_Update(GameData* gameData);
void Game_Shutdown(GameData* gameData);

#endif