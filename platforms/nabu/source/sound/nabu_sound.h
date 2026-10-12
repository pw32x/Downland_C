#ifndef NABU_SOUND_H
#define NABU_SOUND_H

void nabusound_init(void);
void nabusound_frame(void); // call once a frame at the end of the game loop
void nabusound_play(unsigned char soundIndex); // interrupts playing sound. plays to the end.
void nabusound_playLoop(unsigned char soundIndex);
void nabusound_stopLoop(unsigned char soundIndex);
void nabusound_stopAll(void);

#endif
