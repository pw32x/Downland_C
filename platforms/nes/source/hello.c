/*	simple Hello World, for llvm-mos, for NES
 *  writing to the screen with rendering disabled
 *	using neslib
 *	Doug Fraker 2018
 */

#include <neslib.h>
#include "base_types.h"

#include <stdio.h>
#include <mapper.h>

#include "generated/resources.h"

#define BLACK 0x0f
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30

extern const dl_u8 chamber0_cleanBackground[6144];
extern const dl_u8 chamber0_tileMap[32 * 24];
extern const dl_u8 chamber1_tileMap[32 * 24];
extern const dl_u8 chamber2_tileMap[32 * 24];
extern const dl_u8 chamber3_tileMap[32 * 24];
extern const dl_u8 chamber4_tileMap[32 * 24];
extern const dl_u8 chamber5_tileMap[32 * 24];

RoomResources resources;

int main2(void) 
{
  static const char text[] = "Hello World!"; // zero terminated c string

  static const char palette[16] = 
  {
      0x0f, 0x11, 0x27, 0x30 // black, gray, lt gray, white
  };


  set_prg_bank(5);

  ppu_off(); // screen off

 vram_adr(NAMETABLE_A);
 vram_write(chamber5_tileMap, 32 * 24);

  // load the palettes
  pal_bg(palette);
  pal_spr(palette);

/*
  // set a starting point on the screen
  // vram_adr(NTADR_A(x,y));
  vram_adr(NTADR_A(10, 14)); // screen is 32 x 30 tiles

  for (char i = 0; text[i]; ++i)
    vram_put(text[i]); // this pushes 1 char to the screen
*/

  // vram_adr and vram_put only work with screen off
  // NOTE, you could replace everything between i = 0; and here with...
  // vram_write(text,sizeof(text));
  // does the same thing

    oam_size(1);
 bank_spr(0);
 bank_bg(1);

  ppu_on_all(); //	turn on screen

  char y_position = 0x40; // all share same Y, which increases every frame
  char x_position = 0x88;
  char x_position2 = 0xa0;
  char x_position3 = 0xc0;
    oam_size(1);
 bank_spr(0);
 bank_bg(1);

    printf("%d\n", chamber0_cleanBackground[0]);

    const char metasprite2[] = 
    {
        0, 0, 0x02, 0, 
        8, 0, 0x04, 0,  
        128
    };

  while (1) 
  {
    ppu_wait_nmi(); // wait till beginning of the frame
    // the sprites are pushed from a buffer to the OAM during nmi

    // clear all sprites from sprite buffer
    oam_clear();

    oam_meta_spr(x_position3, y_position, metasprite2);    
    
    ++y_position;
  }
}
