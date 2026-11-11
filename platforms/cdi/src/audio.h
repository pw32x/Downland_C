#ifndef __AUDIO_H__
#define	__AUDIO_H__

#include "dl_sound.h"

void initAudio(void);
void closeAudio(void);

dl_s32 loadSoundEffects(dl_s32 file);


#endif
