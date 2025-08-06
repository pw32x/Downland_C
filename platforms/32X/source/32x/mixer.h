#ifndef MIXER_INCLUDE_H
#define MIXER_INCLUDE_H

typedef struct
{
    const int8_t *data;         // points to sample data rom, or 0 for channel off
    uint32_t position;          // current position in the data (fixed point 18.14)
    uint32_t increment;         // step size (fixed point 18.14) for pitch
    uint32_t length;            // size of data (fixed point 18.14)
    uint32_t loop_length;       // size of section to loop (fixed point 18.14) or 0 for no loop
    int8_t   volume;            // 0 to 64
    uint8_t  pan;               // 0 = left, 255 = right
    uint8_t  pad[2];            // pad to 24 bytes
} mixer_t;

/* COMM6 - mixer status */
#define MIXER_INITIALIZE    0
#define MIXER_UNLOCKED      1
#define MIXER_LOCK_MSH2     2
#define MIXER_LOCK_SSH2     3


void MixSamples(mixer_t *mixer, int16_t *buffer, int32_t cnt, int32_t scale);
void LockMixer(int16_t id);
void UnlockMixer(void);

#endif
