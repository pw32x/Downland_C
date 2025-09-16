#include "string_utils.h"

#include "base_defines.h"

void convertTimerToString(dl_u16 timerValue, dl_u8* timerString)
{
	static const dl_u16 divisors[5] = { 10000, 1000, 100, 10, 1 };

	const dl_u16* divisorsRunner = divisors;
	dl_u8* timerStringRunner = timerString;

    dl_u8 i = 5;

    do
	{
        dl_u8 digit = 0;
        while (timerValue >= *divisorsRunner) 
		{
            timerValue -= *divisorsRunner;
            digit++;
        }

		divisorsRunner++;

        *timerStringRunner = digit;
		timerStringRunner++;
    } while (--i);

	/*
    // strip leading zeros -> spaces
    for (i = 0; i < 4; i++) 
	{
        if (timerString[i] == CHAR_0)
            timerString[i] = CHAR_SPACE;
        else
            break;
    }
	*/
}

void convertScoreToString(dl_u32 score, dl_u8* scoreString)
{
	int loop;

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
	scoreString[6] = (dl_u8)score;

	for (loop = 0; loop < SCORE_STRING_SIZE - 2; loop++)
	{
		if (scoreString[loop] == CHAR_0)
			scoreString[loop] = CHAR_SPACE;
		else
			break;
	}
}