#include "sound_manager.h"

// game headers
#include "dl_sound.h"

// 3do headers
#include "audio.h"
#include "music.h"

int32 Rate = 0x8000;

dl_u8 m_isLooping[SOUND_NUM_SOUNDS];
dl_u8 m_isPlaying[SOUND_NUM_SOUNDS];
int m_counts[SOUND_NUM_SOUNDS];
int m_countStart[SOUND_NUM_SOUNDS];

Item soundEffects[SOUND_NUM_SOUNDS];
char* soundInstrumentNames[SOUND_NUM_SOUNDS];
Item soundInstruments[SOUND_NUM_SOUNDS];
Item outputInstruments[SOUND_NUM_SOUNDS];
Item soundAttachments[SOUND_NUM_SOUNDS];

void SoundManager_LoadSound(dl_u8 soundIndex, char* path);

void SoundManager_Init()
{
    dl_u8 loop;

    for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
    {
        m_isLooping[loop] = false;
        m_isPlaying[loop] = false;
    }

    SoundManager_LoadSound(SOUND_JUMP, "jump.aiff");
    SoundManager_LoadSound(SOUND_LAND, "land.aiff");
    SoundManager_LoadSound(SOUND_SCREEN_TRANSITION, "transition.aiff");
    SoundManager_LoadSound(SOUND_SPLAT, "splat.aiff");
    SoundManager_LoadSound(SOUND_PICKUP, "pickup.aiff");
    SoundManager_LoadSound(SOUND_RUN, "run.aiff");
    SoundManager_LoadSound(SOUND_CLIMB_UP, "climb_up.aiff");
    SoundManager_LoadSound(SOUND_CLIMB_DOWN, "climb_down.aiff");

    m_countStart[SOUND_JUMP] = 0;
    m_countStart[SOUND_LAND] = 0;
    m_countStart[SOUND_SCREEN_TRANSITION] = 0;
    m_countStart[SOUND_SPLAT] = 0;
    m_countStart[SOUND_PICKUP] = 0;
    m_countStart[SOUND_RUN] = 64;
    m_countStart[SOUND_CLIMB_UP] = 130;
    m_countStart[SOUND_CLIMB_DOWN] = 48;
}

void SoundManager_Play(dl_u8 soundIndex, dl_u8 loop)
{
    if (m_isLooping[soundIndex] && m_isPlaying[soundIndex])
        return;

    m_isPlaying[soundIndex] = true;

    StartInstrumentVA(soundInstruments[soundIndex], AF_TAG_RATE, Rate, TAG_END);

    m_counts[soundIndex] = m_countStart[soundIndex];
    m_isLooping[soundIndex] = loop;
}

void SoundManager_Stop(dl_u8 soundIndex)
{
    if (m_isPlaying[soundIndex])
        StopInstrument(soundInstruments[soundIndex], NULL);

    m_isPlaying[soundIndex] = false;
    m_isLooping[soundIndex] = false;
    m_counts[soundIndex] = m_countStart[soundIndex];
}

void SoundManager_StopAll()
{
    dl_u8 loop;
    for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
    {
        SoundManager_Stop(loop);
    }
}

void SoundManager_PauseAll()
{
    dl_u8 loop;
    for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
    {
        PauseInstrument(soundInstruments[loop]);
    }
}
void SoundManager_ResumeAll()
{
    dl_u8 loop;
    for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
    {
        ResumeInstrument(soundInstruments[loop]);
    }
}

void SoundManager_Update()
{
    dl_u8 loop;
    for (loop = 0; loop < SOUND_NUM_SOUNDS; loop++)
    {
        if (m_isLooping[loop])
        {
            m_counts[loop]--;

            if (m_counts[loop] == 0)
            {
                StopInstrument(soundInstruments[loop], NULL);
                StartInstrumentVA(soundInstruments[loop], AF_TAG_RATE, Rate, TAG_END);

                m_counts[loop] = m_countStart[loop];
            }
        }
    }
}

void SoundManager_LoadSound(dl_u8 soundIndex, char* path)
{
    soundEffects[soundIndex] = LoadSample(path);

    soundInstrumentNames[soundIndex] = SelectSamplePlayer (soundEffects[soundIndex], FALSE);
    soundInstruments[soundIndex] = LoadInstrument (soundInstrumentNames[soundIndex], 0, 100);

    outputInstruments[soundIndex] = LoadInstrument("directout.dsp", 0, 100);
    StartInstrument(outputInstruments[soundIndex], NULL);

    ConnectInstruments(soundInstruments[soundIndex], "Output", outputInstruments[soundIndex], "InputLeft");
    ConnectInstruments(soundInstruments[soundIndex], "Output", outputInstruments[soundIndex], "InputRight");

    soundAttachments[soundIndex] = AttachSample(soundInstruments[soundIndex], soundEffects[soundIndex], 0);
}
