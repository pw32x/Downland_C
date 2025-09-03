#include <stdio.h>
#include "SMSlib.h"

#include "base_types.h"
#include "game_types.h"

#define VDP_ASSETS_BANK 2
#define CHAMBER0_BANK 3
#define CHAMBER1_BANK 4

void* dl_alloc(dl_u32 size)
{
	return NULL;
}

void dl_memset(void* source, dl_u8 value, dl_u16 count)
{

}

void dl_memcpy(void* destination, const void* source, dl_u16 count)
{
}

void Sound_Play(dl_u8 soundIndex, dl_u8 loop)
{
}

void Sound_Stop(dl_u8 soundIndex)
{
}

void Sound_StopAll(void)
{
}


#define TRUE 1
#define FALSE 0

dl_u8 downlandPalette[] = 
{
	0x00, 0x30, 0x0b, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

extern unsigned char const tileSet4bpp[6240];
extern unsigned char const characterFont4bpp[1248];
extern const dl_u16 chamber0_tileMap[];
extern const dl_u16 chamber1_tileMap[];

extern unsigned char const ball4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const bird4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const diamond4bpp[128];  // 4 tiles x 32 bytes
extern unsigned char const door4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const drop4bpp[64]; // 2 tiles x 32 bytes
extern unsigned char const key4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const moneyBag4bpp[128]; // 4 tiles x 32 bytes
extern unsigned char const player4bpp[1280]; // 40 tiles x 32 bytes

void Scroll_InitTilemap(void)
{
	SMS_mapROMBank(CHAMBER0_BANK);
	SMS_loadTileMap(0, 0, chamber0_tileMap, 32 * 24 * 2);
}

void chamber_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
}

void get_ready_room_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{
}

void titleScreen_draw(dl_u8 roomNumber, GameData* gameData, const Resources* resources)
{

}

GameData gameData;
Resources resources;

void main(void)
{
	/* Clear VRAM */
	SMS_VRAMmemsetW(0, 0x0000, 16384);

	SMS_loadBGPalette(downlandPalette);
	SMS_loadSpritePalette(downlandPalette);
  
	// load tiles for background
	SMS_mapROMBank(VDP_ASSETS_BANK);
	SMS_loadTiles(ball4bpp, 256, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(bird4bpp, 256 + 4, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(diamond4bpp, 256 + 8, 128);  // 4 tiles x 32 bytes
	SMS_loadTiles(door4bpp, 256 + 12, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(drop4bpp, 256 + 16, 64); // 2 tiles x 32 bytes
	SMS_loadTiles(key4bpp, 256 + 18, 1280); // 4 tiles x 32 bytes
	SMS_loadTiles(moneyBag4bpp, 256 + 22, 128); // 4 tiles x 32 bytes
	SMS_loadTiles(player4bpp, 256 + 64, 1280); // 40 tiles x 32 bytes

	SMS_loadTiles(tileSet4bpp, 0, 6240);
	SMS_loadTiles(characterFont4bpp, 195, 1248);
  
	Scroll_InitTilemap();

	/* Turn on the display */
	SMS_displayOn();
	SMS_waitForVBlank ();

	for(;;) 
	{ 
		// Game Loop
		SMS_initSprites();

		// VBLANK
		SMS_waitForVBlank ();

		SMS_copySpritestoSAT();
	}
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"pw","basicsmsproject","A basic SMS example project with devkitSMS");
