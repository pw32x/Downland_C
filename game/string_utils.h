#ifndef STRING_UTILS_INCLUDE_H
#define STRING_UTILS_INCLUDE_H

#include "base_types.h"

#define ROOM_NUMBER_STRING_SIZE 2
#define TIMER_STRING_SIZE 6
#define SCORE_STRING_SIZE 8

void convertTimerToString(u16 timerValue, u8* timerString);
void convertScoreToString(u32 score, u8* scoreString);

#endif