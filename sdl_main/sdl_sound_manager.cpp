#include "sdl_sound_manager.h"

#include "sdl_sound.h"

#include <format>
#include <algorithm>

SDLSoundManager::SDLSoundManager()
	: m_audioDevice(0)
	, m_isPaused(false)
{
}

void SDLSoundManager::init()
{
    if (m_audioDevice)
    {
        throw std::runtime_error(std::format("SDLSoundManager already initialized"));
    }

    if (!SDL_WasInit(SDL_INIT_AUDIO) && !SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        throw std::runtime_error(std::format("Couldn't init audio subsystem: {}", SDL_GetError()));
    }

    m_audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (m_audioDevice == 0) 
    {
        throw std::runtime_error(std::format("Couldn't open audio device: {}", SDL_GetError()));
    }
}

void SDLSoundManager::shutdown()
{
    if (!m_audioDevice)
        return;

    stopAll();
    SDL_CloseAudioDevice(m_audioDevice);
	m_audioDevice = 0;
	m_isPaused = false;
    m_sounds.clear();
}

void SDLSoundManager::loadSound(const char* filename)
{
    if (!m_audioDevice)
    {
        throw std::runtime_error(std::format("SDLSoundManager not initialized"));
    }

    m_sounds.emplace_back(std::make_unique<SDLSound>(filename, m_audioDevice));
}

void SDLSoundManager::play(int soundIndex)
{
    if (!m_audioDevice)
    {
        throw std::runtime_error(std::format("SDLSoundManager not initialized"));
    }

    if (soundIndex > m_sounds.size() - 1)
        return;

    m_sounds[soundIndex]->play();
}

void SDLSoundManager::pause()
{
    if (!m_audioDevice)
    {
        throw std::runtime_error(std::format("SDLSoundManager not initialized"));
    }

    if (m_isPaused)
        return;

    m_isPaused = true;
    for (auto& sound : m_sounds)
    {
        sound->pause();
    }
}
void SDLSoundManager::resume()
{
    if (!m_audioDevice)
    {
        throw std::runtime_error(std::format("SDLSoundManager not initialized"));
    }

    if (!m_isPaused)
        return;

    m_isPaused = false;
    for (auto& sound : m_sounds)
    {
        sound->resume();
    };
}

void SDLSoundManager::stopAll()
{
    if (!m_audioDevice)
    {
        throw std::runtime_error(std::format("SDLSoundManager not initialized"));
    }

    for (auto& sound : m_sounds)
    {
        sound->stop();
    };
}