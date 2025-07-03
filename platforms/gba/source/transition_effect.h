#ifndef TRANSITION_EFFECT_INCLUDE_H
#define TRANSITION_EFFECT_INCLUDE_H

#include "..\..\..\game\base_types.h"

#define TRANSITION_TURN_ON		0x00
#define TRANSITION_OFF			0xff
#define TRANSITION_BLACK_SCREEN	0xfe

extern dl_u8 g_transitionCounter;
extern dl_s32 g_transitionHDMATable[160];

void updateTransitionHDMATable();
void setupTransitionHDMA();

#endif