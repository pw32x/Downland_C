#include "dl_sound.h"

typedef struct
{
    const int8_t *filePtr;
    unsigned int fileSize;
    unsigned int sampleRate;
} SoundFile;

extern SoundFile soundFiles[SOUND_NUM_SOUNDS];