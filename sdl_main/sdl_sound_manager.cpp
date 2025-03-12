#include "sdl_sound_manager.h"

#include <memory>

extern "C" 
{
#include "../game/sound.h"
}

#include "sdl_sound.h"

#include <SDL3/SDL.h>

#include <vector>

SDL_AudioDeviceID gAudioDevice = 0;
bool gPaused = false;
std::vector<std::unique_ptr<SDLSound>> gSounds;

// implement the sound function here
void Sound_Play(u8 soundIndex)
{
	int a = 3;
}

void SDLSoundManager_Init()
{

}

void SDLSoundManager_Shutdown()
{

}
