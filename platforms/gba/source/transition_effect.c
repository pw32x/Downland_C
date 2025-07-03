#include "transition_effect.h"

#include <gba_dma.h>
#include <gba_video.h>

dl_u8 g_transitionCounter = TRANSITION_OFF;
dl_s32 g_transitionHDMATable[160];


void updateTransitionHDMATable() 
{
	if (g_transitionCounter == TRANSITION_OFF)
	{
		// set to zero
		for (int i = 0; i < 160; i++) 
		{
			g_transitionHDMATable[i] = 0;
		}
	}
	else if (g_transitionCounter == TRANSITION_BLACK_SCREEN)
	{
		for (int i = 0; i < 160; i++) 
		{
			g_transitionHDMATable[i] = 1 << 8;
		}
	}
	else
	{
		for (int i = 0; i < 160; i++) 
		{
			dl_u8 index = (i + 1) % 160;
			dl_u8 currentLine = index % 32;

			dl_u16 value = 2; // default blue line

			if (currentLine > g_transitionCounter)
			{
				value = 1; // show non-transparent black line
			}
			else if (currentLine < g_transitionCounter)
			{
				value = 0; // transparent black line
			}

			g_transitionHDMATable[i] = value << 8; 
		}
	}
}

void setupTransitionHDMA() 
{
    REG_DMA3CNT = 0;

    REG_DMA3SAD = (u32)g_transitionHDMATable;
    REG_DMA3DAD = (u32)&REG_BG2Y;
    REG_DMA3CNT = DMA_ENABLE | 
                  DMA_HBLANK | // don't use DMA_SPECIAL
                  DMA_REPEAT | 
                  DMA_DST_FIXED |
                  DMA_SRC_INC | 
                  DMA32 |     // 32 bits at a time into the register.
                  DMA_HBLANK |
                  1; // sending 32 bits one at a time. This is not the total size of the array.
}