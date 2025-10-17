#include "string_utils.h"

#include "base_defines.h"

void convertNumberToString(dl_u32 value, dl_u8* string, dl_u8 length)
{
	int loop;
	dl_u8 index = 0;

	switch (length)
	{
	case 8:
	string[index++] = value / 1000000;
	value %= 1000000;
	case 7:
	string[index++] = value / 100000;
	value %= 100000;
	case 6:
	string[index++] = value / 10000;
	value %= 10000;
	case 5:
	string[index++] = value / 1000;
	value %= 1000;
	case 4:
	string[index++] = value / 100;
	value %= 100;
	case 3:
	string[index++] = value / 10;
	value %= 10;
	case 2:
	string[index++] = (dl_u8)value;
	}

	length -= 2;

	for (loop = 0; loop < length; loop++)
	{
		if (string[loop] == CHAR_0)
			string[loop] = CHAR_SPACE;
		else
			break;
	}	
} 


void convertTimerToString(dl_u16 timerValue, dl_u8* timerString)
{
	convertNumberToString(timerValue, timerString, TIMER_STRING_SIZE);
}

void convertScoreToString(dl_u32 score, dl_u8* scoreString)
{
	convertNumberToString(score, scoreString, SCORE_STRING_SIZE);
}