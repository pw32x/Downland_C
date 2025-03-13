#ifndef SDL_SOUND_MANAGER_INCLUDE_H
#define SDL_SOUND_MANAGER_INCLUDE_H

#include "sdl_sound.h"

#include <SDL3/SDL.h>
#include <vector>
#include <memory>

// super simple SDL sound manager
class SDLSoundManager
{
public:
	SDLSoundManager();

	void init();
	void shutdown();

	void loadSound(const char* filename);

	void play(int soundIndex);
	void stopAll();

	void pause();
	void resume();

private:
	SDL_AudioDeviceID m_audioDevice;
	bool m_isPaused;
	std::vector<std::unique_ptr<SDLSound>> m_sounds;
};

#endif