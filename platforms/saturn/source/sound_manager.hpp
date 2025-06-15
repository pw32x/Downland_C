#ifndef SOUND_MANAGER_INCLUDE_H
#define SOUND_MANAGER_INCLUDE_H

#include <srl.hpp>

#include "base_types.h"
#include "dl_sound.h"

#define SOUND_CHANNEL_COUNT 4

int COUNT = 130;

class SoundManager
{
public:
    SoundManager();

    void Play(u8 soundIndex, bool loop);
    void Stop(u8 soundIndex);

    void Update();

private:
    void LoadSound(u8 soundIndex, const char* path);

public:
    bool m_isLooping[SOUND_NUM_SOUNDS];
    bool m_isPlaying[SOUND_NUM_SOUNDS];
    int m_counts[SOUND_NUM_SOUNDS];
    int m_countStart[SOUND_NUM_SOUNDS];
    SRL::Sound::Pcm::WaveSound* m_sounds[SOUND_NUM_SOUNDS];
};

SoundManager::SoundManager()
{
    for (int loop = 0; loop < SOUND_CHANNEL_COUNT; loop++)
    {
        m_isLooping[loop] = false;
        m_isPlaying[loop] = false;
    }

    LoadSound(SOUND_JUMP,"JUMP.WAV");
    LoadSound(SOUND_LAND,"LAND.WAV");
    LoadSound(SOUND_SCREEN_TRANSITION,"TRNSITIN.WAV");
    LoadSound(SOUND_SPLAT,"SPLAT.WAV");
    LoadSound(SOUND_PICKUP,"PICKUP.WAV");
    LoadSound(SOUND_RUN,"RUN.WAV");
    LoadSound(SOUND_CLIMB_UP,"CLIMB_UP.WAV");
    LoadSound(SOUND_CLIMB_DOWN,"CLIMB_DN.WAV");

    m_countStart[SOUND_JUMP] = 0;
    m_countStart[SOUND_LAND] = 0;
    m_countStart[SOUND_SCREEN_TRANSITION] = 0;
    m_countStart[SOUND_SPLAT] = 0;
    m_countStart[SOUND_PICKUP] = 0;
    m_countStart[SOUND_RUN] = 64;
    m_countStart[SOUND_CLIMB_UP] = 130;
    m_countStart[SOUND_CLIMB_DOWN] = 48;
}

void SoundManager::Play(u8 soundIndex, bool loop)
{
    if (m_sounds[soundIndex] == NULL)
        return;

    if (m_isLooping[soundIndex] && m_isPlaying[soundIndex])
        return;

    m_isPlaying[soundIndex] = true;
    SRL::Sound::Pcm::StopSound(soundIndex);
    m_sounds[soundIndex]->PlayOnChannel(soundIndex);
    m_counts[soundIndex] = m_countStart[soundIndex];
    m_isLooping[soundIndex] = loop;
}

void SoundManager::Stop(u8 soundIndex)
{
    if (m_isPlaying[soundIndex])
        SRL::Sound::Pcm::StopSound(soundIndex);

    m_isPlaying[soundIndex] = false;
    m_isLooping[soundIndex] = false;
    m_counts[soundIndex] = m_countStart[soundIndex];
}

void SoundManager::Update()
{
    for (int loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
    {
        if (m_isLooping[loop])
        {
            m_counts[loop]--;

            if (m_counts[loop] == 0)
            {
                SRL::Sound::Pcm::StopSound(loop);
                m_sounds[loop]->PlayOnChannel(loop);    
                m_counts[loop] = m_countStart[loop];
                //m_counts[loop] = COUNT;
            }
        }
    }
}

void SoundManager::LoadSound(u8 soundIndex, const char* path)
{
    m_sounds[soundIndex] = lwnew SRL::Sound::Pcm::WaveSound(path);
}

#endif