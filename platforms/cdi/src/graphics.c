#include <ucm.h>
#include <memory.h>
#include "graphics.h"

u_int frameDone = 0;
u_int frameTick = 0;

u_char* activeVideoBuffer;
u_char* drawVideoBuffer;

void fillBuffer(register u_int *buffer,
				register u_int data, 
				register u_int size)
{
	int i;
	size = size >> 2;

	for (i = 0; i < size; i++) 
	{
		*buffer++ = data;
	}
}

void fillVideoBuffer(u_char *videoBuffer,
					 u_int data)
{
	fillBuffer((u_int*)videoBuffer, data, VBUFFER_SIZE);
}

void setIcf(register int icfA, 
			register int icfB)
{
	int curIcfA = icfA > ICF_MAX ? ICF_MAX : (icfA < ICF_MIN ? ICF_MIN : icfA);
	int curIcfB = icfB > ICF_MAX ? ICF_MAX : (icfB < ICF_MIN ? ICF_MIN : icfB);

	dc_wrli(videoPath, lctA, 1, 7, cp_icf(PA, curIcfA));
	dc_wrli(videoPath, lctB, 1, 7, cp_icf(PB, curIcfB));
}

void createVideoBuffers()
{
	setIcf(ICF_MAX, ICF_MAX);
	activeVideoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO1);
	drawVideoBuffer = (u_char*)srqcmem(VBUFFER_SIZE, VIDEO1);
	
	fillVideoBuffer(activeVideoBuffer, 0);
	fillVideoBuffer(drawVideoBuffer, 0);

	dc_wrli(videoPath, lctA, 0, 1, cp_dadr((int)activeVideoBuffer + pixelStart));
	dc_wrli(videoPath, lctA, 0, 0, cp_sig());
}


void swapVideoBuffers()
{
	u_char* temp = drawVideoBuffer;
	drawVideoBuffer = activeVideoBuffer;
	activeVideoBuffer = temp;
	dc_wrli(videoPath, lctA, 0, 1, cp_dadr((int)activeVideoBuffer + pixelStart));
}

void initGraphics()
{
	createVideoBuffers();
}

void handleVideoSignal(int sigCode)
{
	if (sigCode == SIG_BLANK)
	{
		frameDone = 1;
		frameTick++;
		dc_ssig(videoPath, SIG_BLANK, 0);
	}
}