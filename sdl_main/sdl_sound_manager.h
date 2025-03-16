#ifndef SDL_SOUND_MANAGER_INCLUDE_H
#define SDL_SOUND_MANAGER_INCLUDE_H

#include "sdl_sound.h"

#include <SDL3/SDL.h>
#include <vector>
#include <memory>

// Super simple SDL sound manager. Sounds are loaded and given
// an index which is referred to by play() and stop();
class SDLSoundManager
{
public:
	SDLSoundManager();

	void init();
	void shutdown();

	int loadSound(const char* filename);

	void play(int soundIndex, bool loop);
	void stop(int soundIndex);

	void stopAll();
	void pauseAll();
	void resumeAll();

private:
	SDL_AudioDeviceID m_audioDevice;
	bool m_isPaused;
	std::vector<std::unique_ptr<SDLSound>> m_sounds;
};

#endif