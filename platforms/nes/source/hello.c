/*	simple Hello World, for llvm-mos, for NES
 *  writing to the screen with rendering disabled
 *	using neslib
 *	Doug Fraker 2018
 */

#include <neslib.h>

#include "base_types.h"

#define BLACK 0x0f
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30

const unsigned char chamber0_tileMap[32 * 24] = 
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2, 0, 0, 0, 0, 0, 
    0, 0, 3, 4, 5, 6, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 9, 5, 6, 7, 8, 7, 8, 7, 8, 7, 9, 10, 0, 0, 0, 
    0, 11, 12, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 16, 0, 0, 
    0, 17, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 
    0, 18, 0, 0, 13, 0, 0, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 0, 0, 
    0, 22, 0, 0, 13, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 18, 0, 0, 13, 0, 0, 0, 24, 25, 26, 27, 26, 27, 26, 27, 26, 27, 26, 27, 26, 27, 26, 27, 26, 28, 29, 30, 29, 0, 0, 0, 
    0, 31, 0, 0, 13, 0, 0, 0, 0, 0, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 32, 33, 34, 35, 36, 37, 0, 0, 
    0, 18, 0, 0, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 39, 0, 0, 
    0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 41, 0, 0, 
    0, 42, 43, 43, 43, 43, 43, 44, 0, 45, 46, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 0, 0, 13, 0, 0, 47, 0, 0, 
    0, 0, 0, 0, 0, 0, 48, 49, 0, 0, 50, 29, 0, 0, 29, 0, 29, 0, 29, 0, 0, 0, 48, 49, 0, 0, 13, 0, 0, 51, 0, 0, 
    0, 0, 52, 53, 54, 55, 35, 0, 0, 0, 0, 56, 54, 55, 57, 58, 57, 58, 57, 59, 54, 55, 35, 0, 0, 0, 13, 0, 0, 60, 0, 0, 
    0, 61, 62, 0, 13, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 35, 0, 35, 0, 35, 13, 0, 0, 0, 0, 0, 63, 0, 0, 64, 0, 0, 
    0, 65, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 65, 0, 0, 
    0, 66, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 67, 0, 0, 0, 0, 0, 0, 0, 67, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 
    0, 68, 0, 0, 13, 0, 0, 68, 69, 70, 0, 0, 0, 0, 0, 68, 69, 70, 0, 0, 0, 0, 0, 68, 69, 70, 0, 0, 0, 68, 0, 0, 
    0, 18, 0, 0, 13, 0, 0, 71, 0, 18, 0, 0, 0, 0, 0, 71, 0, 18, 0, 0, 0, 0, 0, 71, 0, 18, 0, 0, 0, 71, 0, 0, 
    0, 17, 0, 0, 13, 0, 0, 15, 72, 73, 0, 0, 0, 0, 0, 15, 72, 73, 0, 0, 0, 0, 0, 15, 72, 73, 0, 0, 0, 74, 0, 0, 
    0, 18, 0, 0, 75, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 75, 0, 0, 0, 0, 76, 0, 0, 
    0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 0, 0, 
    0, 78, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 80, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};



int main(void) {
  static const char text[] = "Hello World!"; // zero terminated c string

  static const char palette[16] = 
  {
      0x0f, 0x11, 0x27, 0x30 // black, gray, lt gray, white
  };


  ppu_off(); // screen off

 vram_adr(NAMETABLE_A);
 vram_write(chamber0_tileMap, 32 * 24);

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
