#ifndef SDL_SOUND_INCLUDE_H
#define SDL_SOUND_INCLUDE_H

#include <SDL3/SDL.h>

// barebones support for SDL playback of a sound
class SDLSound
{
public:
    SDLSound(const char* filename, SDL_AudioDeviceID audio_device);
    ~SDLSound();
    bool isPaused() { return m_isPaused; }
    bool isLooped() { return m_isLooped; }
    void play(bool loop);
    void stop();
    void pause();
    void resume();

    Uint32 getWavDataLength() const { return m_wavDataLength; }
    const Uint8* getWavData() const { return m_wavData; }

protected:
    bool m_isPaused;
    bool m_isLooped;
    Uint8* m_wavData;
    Uint32 m_wavDataLength;
    SDL_AudioStream* m_audioStream;
    SDL_AudioSpec m_sourceAudioSpec;
    SDL_AudioSpec m_destinationAudioSpec;
};

#endif