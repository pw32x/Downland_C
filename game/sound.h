#ifndef SOUND_INCLUDE_H
#define SOUND_INCLUDE_H

#include "base_types.h"

#define SOUND_JUMP				0
#define SOUND_LAND				1
#define SOUND_SCREEN_TRANSITION	2
#define SOUND_SPLAT				3
#define SOUND_PICKUP			4
#define SOUND_NUM_SOUNDS		5

// declares a function to play a sound. It's up to the
// platform implementation to do the loading of the sound
// and implement this function.
void Sound_Play(u8 sound_id);

#endif