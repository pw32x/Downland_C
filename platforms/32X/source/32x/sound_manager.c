#include "sound_manager.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "32x.h"
#include "marshw.h"
#include "mixer.h"

#define SAMPLE_RATE    22050
#define SAMPLE_MIN         2
#define SAMPLE_CENTER    517
#define SAMPLE_MAX      1032
#define MAX_NUM_SAMPLES 1024

#define NUM_SFX_MIXERS 4

int16_t __attribute__((aligned(16))) g_soundBuffer0[MAX_NUM_SAMPLES * 2];
int16_t __attribute__((aligned(16))) g_soundBuffer1[MAX_NUM_SAMPLES * 2];
int16_t* g_currentSoundBuffer;

uint16_t g_samplesCount = 441;

static mixer_t __attribute__((aligned(16))) sfx_mixers[NUM_SFX_MIXERS];

void update_sound_buffer(int16_t *soundBuffer);
void SoundManager_updateDma();


void SoundManager_init()
{
    uint16_t sample, ix;

    // init DMA
    SH2_DMA_SAR0 = 0;
    SH2_DMA_DAR0 = 0;
    SH2_DMA_TCR0 = 0;
    SH2_DMA_CHCR0 = 0;
    SH2_DMA_DRCR0 = 0;
    SH2_DMA_SAR1 = 0;
    SH2_DMA_DAR1 = 0x20004034; // storing a long here will set left and right
    SH2_DMA_TCR1 = 0;
    SH2_DMA_CHCR1 = 0;
    SH2_DMA_DRCR1 = 0;
    SH2_DMA_DMAOR = 1; // enable DMA

    SH2_DMA_VCR1 = 72; // set exception vector for DMA channel 1
    SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0F00; // set DMA INT to priority 15

    // init the sound hardware
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
        MARS_PWM_CYCLE = (((23011361 << 1)/SAMPLE_RATE + 1) >> 1) + 1; // for NTSC clock
    else
        MARS_PWM_CYCLE = (((22801467 << 1)/SAMPLE_RATE + 1) >> 1) + 1; // for PAL clock
    MARS_PWM_CTRL = 0x0185; // TM = 1, RTP, RMD = right, LMD = left

    sample = SAMPLE_MIN;
    /* ramp up to SAMPLE_CENTER to avoid click in audio (real 32X) */
    while (sample < SAMPLE_CENTER)
    {
        for (ix=0; ix<(SAMPLE_RATE*2)/(SAMPLE_CENTER - SAMPLE_MIN); ix++)
        {
            while (MARS_PWM_MONO & 0x8000) ; // wait while full
            MARS_PWM_MONO = sample;
        }
        sample++;
    }

    // initialize mixer
    MARS_SYS_COMM6 = MIXER_UNLOCKED; // sound subsystem running
    g_currentSoundBuffer = g_soundBuffer0;
    update_sound_buffer(g_currentSoundBuffer); // fill first buffer
    SoundManager_updateDma(); // start DMA

    memset(sfx_mixers, 0, sizeof(sfx_mixers));

    SetSH2SR(2);
}

void SoundManager_waitUntilInitialized()
{
    while (MARS_SYS_COMM6 != MIXER_UNLOCKED) ; // wait for sound subsystem to init
}

static inline void flush_mem(void *ptr, int32_t len)
{
    while (len > 0)
    {
        CacheClearLine(ptr);
        ptr += 16;
        len -= 16;
    }
}

static void update_soundeffects(int16_t *soundBuffer)
{
    uint8_t i;

    for (i = 0; i < NUM_SFX_MIXERS; i++)
    {
        CacheClearLine(&sfx_mixers[i].data);

        if (sfx_mixers[i].data)
        {
            MixSamples(&sfx_mixers[i], soundBuffer, g_samplesCount, 64);
        }
    }
}

void update_sound_buffer(int16_t *soundBuffer)
{
    int16_t i;

    memset(soundBuffer, 0, g_samplesCount * 4);

    LockMixer(MIXER_LOCK_SSH2);
    update_soundeffects(soundBuffer);
    UnlockMixer();

    // convert buffer from s16 pcm samples to u16 pwm samples
    for (i = 0; i < g_samplesCount * 2; i++)
    {
        int16_t s = *soundBuffer + SAMPLE_CENTER;
        *soundBuffer++ = (s < 0) ? SAMPLE_MIN : (s > SAMPLE_MAX) ? SAMPLE_MAX : s;
    }
}


SoundHandle SoundManager_playSoundEffect(const int8_t *data, 
                                         uint32_t dataSize, 
                                         uint16_t sampleRate, 
                                         uint8_t loop,
                                         uint8_t volume, 
                                         uint8_t pan)
{
#define SETUP_SOUND \
    sfx_mixers[i].position = 0;\
    sfx_mixers[i].increment = (sampleRate << 14) / SAMPLE_RATE;\
    sfx_mixers[i].length = dataSize << 14;\
    sfx_mixers[i].loop_length = loop ? (dataSize - 1) << 14 : 0;\
    sfx_mixers[i].volume = volume;\
    sfx_mixers[i].pan = pan;\
    UnlockMixer();

    SoundHandle i;

    LockMixer(MIXER_LOCK_MSH2);

    // look for free mixer
    for (i = 0; i < NUM_SFX_MIXERS; i++)
    {
        CacheClearLine(&sfx_mixers[i].data);
        if (!sfx_mixers[i].data)
        {
            // found free mixer
            sfx_mixers[i].data = (const int8_t*)((uint32_t)data | 0x20000000);
            SETUP_SOUND;

            return i;
        }
    }

    // didn't find a free mixer. Maybe the sound
    // was already playing. If so, restart it.
    for (i = 0; i < NUM_SFX_MIXERS; i++)
    {
        if (sfx_mixers[i].data == (const int8_t*)((uint32_t)data | 0x20000000))
        {
            // found same sfx - restart
            SETUP_SOUND;

            return i;
        }
    }

    UnlockMixer();
    return INVALID_SOUND_HANDLE; // failed
}

void SoundManager_changeSoundEffectParams(SoundHandle soundHandle, int32_t srate, int32_t volume, int32_t pan)
{
    if (soundHandle > NUM_SFX_MIXERS - 1)
    {
        return;
    }

    LockMixer(MIXER_LOCK_MSH2);

    CacheClearLine(&sfx_mixers[soundHandle].data);
    if (sfx_mixers[soundHandle].data)
    {
        // still playing - update parameters
        if (srate != -1)
            sfx_mixers[soundHandle].increment = (srate << 14) / SAMPLE_RATE;
        if (volume != -1)
            sfx_mixers[soundHandle].volume = volume;
        if (volume != -1)
            sfx_mixers[soundHandle].pan = pan;
    }

    UnlockMixer();
}

void SoundManager_stopSoundEffect(SoundHandle soundHandle)
{
    if (soundHandle < NUM_SFX_MIXERS)
    {
        LockMixer(MIXER_LOCK_MSH2);

        CacheClearLine(&sfx_mixers[soundHandle].data);
        sfx_mixers[soundHandle].data = 0;

        UnlockMixer();
    }
}

uint8_t SoundManager_isSoundEffectPlaying(SoundHandle soundHandle)
{
    uint8_t res = 0;

    if (soundHandle < NUM_SFX_MIXERS)
    {
        LockMixer(MIXER_LOCK_MSH2);

        CacheClearLine(&sfx_mixers[soundHandle].data);
        if (sfx_mixers[soundHandle].data)
            res = 1;

        UnlockMixer();
    }

    return res;
}

void SoundManager_shutdown()
{
    LockMixer(MIXER_LOCK_MSH2); // locked - stop playing
}

void SoundManager_updateDma()
{
    while (MARS_SYS_COMM6 == MIXER_LOCK_MSH2) ; // locked by MSH2

    SH2_DMA_CHCR1; // read TE
    SH2_DMA_CHCR1 = 0; // clear TE

    // send the current buffer to DMA and switch
    // to the other buffer to fill.
    if (g_currentSoundBuffer == g_soundBuffer0)
    {
        SH2_DMA_SAR1 = ((uint32_t)g_soundBuffer0) | 0x20000000;
        g_currentSoundBuffer = g_soundBuffer1;
    }
    else
    {
        SH2_DMA_SAR1 = ((uint32_t)g_soundBuffer1) | 0x20000000;
        g_currentSoundBuffer = g_soundBuffer0;
    }

    SH2_DMA_TCR1 = g_samplesCount; // number longs
    SH2_DMA_CHCR1 = 0x18E5; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled        

    update_sound_buffer(g_currentSoundBuffer);
}

// helper function for libxmp
void* xmp_malloc(int32_t size)
{
    // make sure cache line size aligned
    return (void*)memalign(16, size);
}
