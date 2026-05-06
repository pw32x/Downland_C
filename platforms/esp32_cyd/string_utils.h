#ifndef STRING_UTILS_INCLUDE_H
#define STRING_UTILS_INCLUDE_H

#include "base_types.h"

#define ROOM_NUMBER_STRING_SIZE 2
#define TIMER_STRING_SIZE 6
#define SCORE_STRING_SIZE 8

#define CHAR_0		0
#define CHAR_1		1
#define CHAR_2		2
#define CHAR_3		3
#define CHAR_4		4
#define CHAR_5		5
#define CHAR_6		6
#define CHAR_7		7
#define CHAR_8		8
#define CHAR_9		9
#define CHAR_SPACE 0x24

void convertTimerToString(dl_u16 timerValue, dl_u8* timerString);
void convertScoreToString(dl_u32 score, dl_u8* scoreString);

#endif