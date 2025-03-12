#ifndef SDL_SOUND_INCLUDE_H
#define SDL_SOUND_INCLUDE_H

#include <SDL3/SDL.h>

class SDLSound
{
public:
    SDLSound(const char* filename, SDL_AudioDeviceID audio_device);
    ~SDLSound();
    bool isPaused() { return m_isPaused; }
    void play();
    void stop();
    void pause();
    void resume();

protected:
    bool m_isPaused;
    Uint8* m_wavData;
    Uint32 m_wavDataLength;
    SDL_AudioStream* m_audioStream;
    SDL_AudioSpec m_sourceAudioSpec;
    SDL_AudioSpec m_destinationAudioSpec;
};

#endif