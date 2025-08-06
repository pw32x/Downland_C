#ifndef SOUND_MANAGER_INCLUDE_H
#define SOUND_MANAGER_INCLUDE_H

#include <stdint.h>

extern int8_t g_masterVolume;
extern const int8_t g_maxMasterVolume;

#define INVALID_SOUND_HANDLE 0xFF
typedef uint8_t SoundHandle;

// call on the secondary CPU
void SoundManager_init();
void SoundManager_waitUntilInitialized();
void SoundManager_shutdown();

// sound effect functions
SoundHandle SoundManager_playSoundEffect(const int8_t *data, 
                                         uint32_t dataSize, 
                                         uint16_t sampleRate, 
                                         uint8_t loop,
                                         uint8_t volume, 
                                         uint8_t pan);

void SoundManager_stopSoundEffect(SoundHandle soundHandle);

void SoundManager_changeSoundEffectParams(SoundHandle soundHandle, 
                                          int32_t sampleRate, 
                                          int32_t volume, 
                                          int32_t pan);

uint8_t SoundManager_isSoundEffectPlaying(SoundHandle soundHandle);

#endif
