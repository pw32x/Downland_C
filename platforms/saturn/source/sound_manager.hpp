#ifndef SOUND_MANAGER_INCLUDE_H
#define SOUND_MANAGER_INCLUDE_H

#include <srl.hpp>

#include "base_types.h"
#include "dl_sound.h"

#define SOUND_CHANNEL_COUNT 4

class SoundManager
{
public:
    SoundManager();

    void Play(u8 soundIndex, bool loop);
    void Stop(u8 soundIndex);

    void Update();

private:
    void LoadSound(u8 soundIndex, const char* path);

private:
    u8 m_currentChannel;
    bool m_activeChannels[SOUND_CHANNEL_COUNT];
    bool m_looping[SOUND_CHANNEL_COUNT];
    u8 m_channelToSoundIndex[SOUND_CHANNEL_COUNT];
    u8 m_soundIndexToChannel[SOUND_NUM_SOUNDS];
    SRL::Sound::Pcm::WaveSound* m_sounds[SOUND_NUM_SOUNDS];
};

SoundManager::SoundManager()
    : m_currentChannel(0)
{
    for (int loop = 0; loop < SOUND_CHANNEL_COUNT; loop++)
        m_looping[loop] = false;

    LoadSound(SOUND_JUMP,"JUMP.WAV");
    LoadSound(SOUND_LAND,"LAND.WAV");
    LoadSound(SOUND_SCREEN_TRANSITION,"TRNSITIN.WAV");
    LoadSound(SOUND_SPLAT,"SPLAT.WAV");
    LoadSound(SOUND_PICKUP,"PICKUP.WAV");
    LoadSound(SOUND_RUN,"RUN.WAV");
    LoadSound(SOUND_CLIMB_UP,"CLIMB_UP.WAV");
    LoadSound(SOUND_CLIMB_DOWN,"CLIMB_DN.WAV");
}

void SoundManager::Play(u8 soundIndex, bool loop)
{
    m_sounds[soundIndex]->PlayOnChannel(m_currentChannel);
    m_looping[m_currentChannel] = loop;
    m_channelToSoundIndex[m_currentChannel] = soundIndex;
    m_soundIndexToChannel[soundIndex] = m_currentChannel;
    m_currentChannel = (m_currentChannel + 1) % SOUND_CHANNEL_COUNT;
}

void SoundManager::Stop(u8 soundIndex)
{
    u8 channel = m_soundIndexToChannel[soundIndex];
    SRL::Sound::Pcm::StopSound(channel);
    m_looping[channel] = false;
}

void SoundManager::Update()
{
    for (int loop = 0; loop < SOUND_CHANNEL_COUNT; loop++)
    {
        if (SRL::Sound::Pcm::IsChannelFree(loop) &&
            m_looping[loop])
        {
            u8 soundIndex = m_channelToSoundIndex[loop];

            m_sounds[soundIndex]->PlayOnChannel(loop);        
        }
    }
}

void SoundManager::LoadSound(u8 soundIndex, const char* path)
{
    m_sounds[soundIndex] = lwnew SRL::Sound::Pcm::WaveSound(path);
}

#endif