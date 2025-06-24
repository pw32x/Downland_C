// --------------------------------------------------------------------
//
//  Super Simple Background Text Scroller v1.0
//
//   Not terribly unlike how we did it in the c64 days.  Although this
//   is C and on the c64 we used asm, the concept is very much similar.
//   This source is set up so you can tweak the speed and the row to
//   scroll the text on...  In actuality there are a number of ways
//   to perform a scroller like this, but I tried to break it down
//   in a simple fashion to make it easier to understand (hopefully!)
//
//   r6502 - 2004-04-02
//
//   Note: tabs == 4 on my setup
//
// --------------------------------------------------------------------


#include <gba_base.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>

#include "r6502_portfont_bin.h"
#include "downland_rom.h"
#include "game_runner.h"
#include "image_utils.h"

#include "..\..\..\game\base_types.h"
#include "..\..\..\game\resource_types.h"
#include "..\..\..\game\resource_loader_buffer.h"
#include "..\..\..\game\checksum_utils.h"
#include "..\..\..\game\game.h"

GameData gameData;
Resources resources;

static dl_u8 memory[18288];
static dl_u8* memoryEnd = NULL;

void* dl_alloc(dl_u32 size)
{
	if (memoryEnd == NULL)
	{
		memoryEnd = memory;
	}

	dl_u8* memory = memoryEnd;

	memoryEnd += size;

	return (void*)memory;
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

// --------------------------------------------------------------------

#define MAPADDRESS		MAP_BASE_ADR(31)	// our base map address
#define DELAY			2			// slow things down
#define TILEWIDTH		8			// how much to scroll
#define ROW				10			// what row to place text at

// --------------------------------------------------------------------

const u16 palette[] = 
{
	RGB8(0x40,0x80,0xc0),
	RGB8(0xFF,0xFF,0xFF),
	RGB8(0xF5,0xFF,0xFF),
	RGB8(0xDF,0xFF,0xF2),
	RGB8(0xCA,0xFF,0xE2),
	RGB8(0xB7,0xFD,0xD8),
	RGB8(0x2C,0x4F,0x8B)
};

// Example 8x8 4bpp sprite data (16 colors, 32 bytes)
// This is just a simple pattern (like a checkerboard)
const unsigned short spriteTiles[16] = 
{
    0x1111, 0x2222, 
	0x1111, 0x2222,
    0x1111, 0x2222, 
	0x1111, 0x2222,
    0x1111, 0x2222, 
	0x1111, 0x2222,
    0x1111, 0x2222, 
	0x1111, 0x2222,
};

// Simple palette with two colors (index 0 is transparent)
const unsigned short spritePalette[16] = 
{
    RGB5(0,0,0),   // Transparent color (palette index 0)
    RGB5(0,0,31),  // Blue
    RGB5(31,20,0), // Orange
    RGB5(31,31,31),// white
    RGB5(31,31,0), // Yellow
    RGB5(31,0,31), // Magenta
    RGB5(0,31,31), // Cyan
    RGB5(31,31,31),// White
    0,0,0,0,0,0,0,0 // rest unused
};

// --------------------------------------------------------------------

const u8 message[] = 
{
	"                                " \
	"Hello, this is an example of an oldschool simple tile scroller " \
	"not unlike how it was done in days of yore.  The '@' symbol " \
	"at the top of your screen is intentional, to dispel the illusion " \
	"of this scroller, to demonstrate the simple concept behind it. " \
	"Check out the source to learn how it works.  It is very simple! " \
	"This exercise brought to you by r6502...          " \
	"Text is about to restart... "
};

// --------------------------------------------------------------------


void updatescrolltext(u32 idx)
{
	u32 i;
	u16 *temppointer;

	temppointer = (u16 *)MAPADDRESS + (ROW * 32);

	// write out a whole row of text to the map
	for (i = 0; i < 32; i++)
	{
		// check for end of message so we can wrap around properly
		if(message[idx] == 0) idx = 0;

		// write a character - we subtract 32, because the font graphics
		// start at tile 0, but our text is in ascii (starting at 32 and up)
		// in other words, tile 0 is a space in our font, but in ascii a
		// space is 32 so we must account for that difference between the two.
		*temppointer++ = message[idx++] - 32;
	}
}


int main() 
{
	if (!checksumCheckLitteEndian(downland_rom, downland_rom_size))
		return -1;
	
	if (!ResourceLoaderBuffer_Init(downland_rom, downland_rom_size, &resources))
		return -1;

	GameRunner_Init(&gameData, &resources);
	
	/*
	dl_u8 downlandSprite[256];
	for (int loop = 0; loop < 256; loop++)
	{
		downlandSprite[loop] = 1;
	}

	downlandSprite[0] = 2;
	downlandSprite[255] = 2;

	downlandSprite[15] = 2;
	//downlandSprite[240] = 2;

	convert1bppImageTo8bppCrtEffectImage(resources.sprites_player,
                                         downlandSprite,
                                         16,
                                         16,
                                         CrtColor_Blue);

	convertToTiles(downlandSprite, 16, 16, 32); // 32 bytes after the last sprite
	*/

	// Set up the interrupt handlers
	irqInit();
	// Enable Vblank Interrupt to allow VblankIntrWait
	irqEnable(IRQ_VBLANK);

	// Allow Interrupts
	REG_IME = 1;

	u32 i, scrollx, scrolldelay, textindex;
	volatile u16 *temppointer;


	// load the palette for the background, 7 colors
	temppointer = BG_COLORS;
	for (i=0; i<7; i++) 
	{
		*temppointer++ = palette[i];
	}

   // Load sprite palette to OBJ palette memory (256 colors max)
    // Palettes are 16 colors per palette, so 16 * 16 bits = 32 bytes
    for (int i = 0; i < 16; i++) 
	{
        SPRITE_PALETTE[i] = spritePalette[i];
    }

	// Load sprite tile data to charblock 4 (OBJ VRAM)
    // Since we are using 1D mapping, tile 0 is at 0x6010000
    // Each tile is 32 bytes for 4bpp 8x8
    //CpuFastSet(spriteTiles, (void*)CHAR_BASE_BLOCK(4), 4 | COPY32);


	/*
	for (int i = 0; i < 128; i++) 
	{
		OAM[i].attr0 = ATTR0_DISABLED;
		OAM[i].attr1 = 0;
		OAM[i].attr2 = 0;
	}

    // Set OAM entry 0:
    // Attribute 0: Y coordinate and shape
    // Attribute 1: X coordinate and size
    // Attribute 2: tile index, priority, palette bank
    OAM[0].attr0 = ATTR0_COLOR_16 | ATTR0_SQUARE | 50; // Y=50
    OAM[0].attr1 = ATTR1_SIZE_8 | 50;                  // X=50
    OAM[0].attr2 = 0;                                  // tile index 0, palette 0


    // Set OAM entry 0:
    // Attribute 0: Y coordinate and shape
    // Attribute 1: X coordinate and size
    // Attribute 2: tile index, priority, palette bank
    OAM[1].attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE | 100; // Y=50
    OAM[1].attr1 = ATTR1_SIZE_16 | 100;                  // X=50
    OAM[1].attr2 = 1;                                  // tile index 0, palette 0
	*/

	// load the font into gba video mem (48 characters, 4bit tiles)

	CpuFastSet(r6502_portfont_bin, (u16*)VRAM,(r6502_portfont_bin_size/4) | COPY32);

	// clear screen map with tile 0 ('space' tile) (256x256 halfwords)

	*((u32 *)MAP_BASE_ADR(31)) =0;
	CpuFastSet( MAP_BASE_ADR(31), MAP_BASE_ADR(31), FILL | COPY32 | (0x800/4));

	// set screen H and V scroll positions
	BG_OFFSET[0].x = 0; BG_OFFSET[0].y = 0;

	// initialize our variables
	scrollx = 0;
	textindex = 0;
	scrolldelay = 0;

	// put the '@' symbol on the top of the screen to show how
	// the screen is only scrolling 7 pixels - to reveal the
	// illusion of how the scroller works
	*((u16 *)MAPADDRESS + 1) = 0x20;	// 0x20 == '@'

	// draw a row of text from beginning of message
	updatescrolltext(0);

	// set the screen base to 31 (0x600F800) and char base to 0 (0x6000000)
	BGCTRL[0] = SCREEN_BASE(31);

	// screen mode & background to display
	SetMode( MODE_0 | BG0_ON | MODE_0 | OBJ_ENABLE | OBJ_1D_MAP);

	while (1) 
	{
		GameRunner_Update(&gameData, &resources);

		VBlankIntrWait();

		GameRunner_Draw(&gameData, &resources);

		// check if we reached our delay
		if (scrolldelay == DELAY) 
		{
			// yes, the delay is complete, so let's reset it
			scrolldelay = 0;

			// check if we reached our scrollcount
			if (scrollx == (TILEWIDTH-1)) 
			{
				// yes, we've scrolled enough, so let's reset the count
				scrollx = 0;

				// check if we reached the end of our scrolltext
				// and if so we need to restart our index
				if (message[textindex] == 0) 
					textindex = 0;
				else 
					textindex++;

				// finally, let's update the scrolltext with the current text index
				updatescrolltext(textindex);
			}
			else 
			{
				scrollx++;
			}
		}
		else 
		{
			scrolldelay++;
		}

		// update the hardware horizontal scroll register
		BG_OFFSET[0].x = scrollx;
	}
}
