#ifndef SOUND_INCLUDE_H
#define SOUND_INCLUDE_H

#include "base_types.h"

#define SOUND_JUMP				0
#define SOUND_LAND				1
#define SOUND_SCREEN_TRANSITION	2
#define SOUND_SPLAT				3
#define SOUND_PICKUP			4
#define SOUND_RUN				5
#define SOUND_CLIMB_UP			6
#define SOUND_CLIMB_DOWN		7
#define SOUND_NUM_SOUNDS		8

// The game does not process sound in any way. It's up to the
// platform implementation to handle the resource loading and
// playback of sounds. Once that is set up, the platform 
// implementation implements the function below which is used by 
// the game to request a sound to play. 
void Sound_Play(dl_u8 soundIndex, dl_u8 loop);
void Sound_Stop(dl_u8 soundIndex);

#endif