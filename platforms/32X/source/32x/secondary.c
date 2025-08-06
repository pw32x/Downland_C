/*
  Victor Luchits

  The MIT License (MIT)

  Copyright (c) 2021 Victor Luchits, Derek John Evans, id Software and ZeniMax Media

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/


#include "32x.h"
#include "mars.h"
#include "sound_manager.h"

// !!! if this is changed, it must be changed in asm too!
typedef void (*setbankpage_t)(int bank, int page);
typedef struct {
	uint16_t bank;
	uint16_t bankpage;
	setbankpage_t setbankpage;
	short *validcount;
	uint16_t pad[2];
} mars_tls_t __attribute__((aligned(16))); // thread local storage



static volatile mars_tls_t mars_tls_pri, mars_tls_sec;
//static uint32_t mars_rom_bsw_start = 0;


void Mars_Secondary(void)
{
	// init thread-local storage
	__asm volatile("mov %0, r0\n\tldc r0,gbr" : : "rm"(&mars_tls_sec) : "r0", "gbr");

	/*
	// init DMA
	SH2_DMA_SAR0 = 0;
	SH2_DMA_DAR0 = 0;
	SH2_DMA_TCR0 = 0;
	SH2_DMA_CHCR0 = 0;
	SH2_DMA_DRCR0 = 0;
	SH2_DMA_SAR1 = 0;
	SH2_DMA_DAR1 = 0;
	SH2_DMA_TCR1 = 0;
	SH2_DMA_CHCR1 = 0;
	SH2_DMA_DRCR1 = 0;
	SH2_DMA_DMAOR = 1; 	// enable DMA

	SH2_DMA_VCR1 = 66; 	// set exception vector for DMA channel 1
	SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0400; // set DMA INT to priority 4

	SetSH2SR(1); 		// allow ints
	*/

	SoundManager_init();

	while (1)
	{
		int cmd;

		while ((cmd = MARS_SYS_COMM4) == MARS_SECCMD_NONE);

		switch (cmd) {
		case MARS_SECCMD_CLEAR_CACHE:
			Mars_ClearCache();
			break;
		case MARS_SECCMD_BREAK:
			// break current command
			break;
		default:
			break;
		}

		MARS_SYS_COMM4 = MARS_SECCMD_NONE;
	}
}


void secondary()
{
	Mars_Secondary();
}

#define	FRACBITS		16

int Mars_FRTCounter2Msec(int c)
{
	return (c * mars_frtc2msec_frac) >> FRACBITS;
}



