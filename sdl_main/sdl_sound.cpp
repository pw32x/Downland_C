#include "sdl_sound.h"

#include <format>

// Sound handling based on SDL3 examples.

void __cdecl SDL_Sound_AudioStreamCallback(void *userdata, 
                                           SDL_AudioStream *stream, 
                                           int additional_amount, 
                                           int total_amount)
{
    SDLSound* sound = static_cast<SDLSound*>(userdata);

    if (!sound->isLooped())
        return;

    int amount = SDL_GetAudioStreamQueued(stream);

    // if there's less than the size of the wave left in the stream
    // feed in another copy of the sound.
    if (amount < (int)sound->getWavDataLength()) 
    {
        /* feed more data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
        SDL_PutAudioStreamData(stream, sound->getWavData(), sound->getWavDataLength());
    }
}

SDLSound::SDLSound(const char* filename, SDL_AudioDeviceID audioDevice) 
 : m_wavData(nullptr),
   m_wavDataLength(0),
   m_audioStream(nullptr),
   m_isPaused(false),
   m_isLooped(false)
{
    SDL_AudioSpec spec;
    char *wav_path = NULL;

    std::string path = std::format("{}{}", SDL_GetBasePath(), filename); 

    if (!SDL_LoadWAV(path.c_str(), &spec, &m_wavData, &m_wavDataLength)) 
    {
        throw std::runtime_error(std::format("Couldn't load .wav file: {}", SDL_GetError()));
    }

    // Create an audio stream. Set the source format to the wav's format (what
    // we'll input), leave the dest format NULL here (it'll change to what the
    // device wants once we bind it).
    m_audioStream = SDL_CreateAudioStream(&spec, NULL);
    if (m_audioStream == nullptr) 
    {
        throw std::runtime_error(std::format("Couldn't create audio stream: {}", SDL_GetError()));
    } 

    if (!SDL_BindAudioStream(audioDevice, m_audioStream)) 
    {  
        throw std::runtime_error(std::format("Failed to bind '{}' stream to device: {}", filename, SDL_GetError()));
    } 

    SDL_GetAudioStreamFormat(m_audioStream, &m_sourceAudioSpec, &m_destinationAudioSpec);
}

SDLSound::~SDLSound()
{
    if (m_audioStream != nullptr) 
        SDL_DestroyAudioStream(m_audioStream);

    if (m_wavData != nullptr)
        SDL_free(m_wavData);
}

// starts the sound from the beginning
void SDLSound::play(bool loop)
{
    m_isPaused = false;
    m_isLooped = loop;

    // just dump the whole sound. we're not being fancy.
    SDL_PutAudioStreamData(m_audioStream, 
                           m_wavData, 
                           (int)m_wavDataLength);

    if (m_isLooped)
    {
        SDL_SetAudioStreamGetCallback(m_audioStream, SDL_Sound_AudioStreamCallback, this);
    }
}

void SDLSound::stop()
{
    m_isPaused = false;
    SDL_ClearAudioStream(m_audioStream);
    SDL_SetAudioStreamGetCallback(m_audioStream, NULL, NULL);
}

void SDLSound::pause()
{
    m_isPaused = true;
    SDL_PauseAudioStreamDevice(m_audioStream);
}

void SDLSound::resume()
{
    m_isPaused = false;
    SDL_ResumeAudioStreamDevice(m_audioStream);
}

