#ifndef SOUND_MANAGER_INCLUDE_H
#define SOUND_MANAGER_INCLUDE_H

#include "base_types.h"

void SoundManager_Init();
void SoundManager_Play(dl_u8 soundIndex, dl_u8 loop);
void SoundManager_Stop(dl_u8 soundIndex);
void SoundManager_StopAll();
void SoundManager_PauseAll();
void SoundManager_ResumeAll();
void SoundManager_Update();

#endif