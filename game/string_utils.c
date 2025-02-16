#include "string_utils.h"

#include "base_defines.h"

void convertTimerToString(u16 timerValue, u8* timerString)
{
	timerString[0] = timerValue / 10000;
	timerValue %= 10000;
	timerString[1] = timerValue / 1000;
	timerValue %= 1000;
	timerString[2] = timerValue / 100;
	timerValue %= 100;
	timerString[3] = timerValue / 10;
	timerValue %= 10;
	timerString[4] = (u8)timerValue;

	for (int loop = 0; loop < TIMER_STRING_SIZE - 2; loop++)
	{
		if (timerString[loop] == CHAR_0)
			timerString[loop] = CHAR_SPACE;
		else
			break;
	}
}

void convertScoreToString(u32 score, u8* scoreString)
{
	scoreString[0] = score / 1000000;
	score %= 1000000;
	scoreString[1] = score / 100000;
	score %= 100000;
	scoreString[2] = score / 10000;
	score %= 10000;
	scoreString[3] = score / 1000;
	score %= 1000;
	scoreString[4] = score / 100;
	score %= 100;
	scoreString[5] = score / 10;
	score %= 10;
	scoreString[6] = (u8)score;

	for (int loop = 0; loop < SCORE_STRING_SIZE - 2; loop++)
	{
		if (scoreString[loop] == CHAR_0)
			scoreString[loop] = CHAR_SPACE;
		else
			break;
	}
}