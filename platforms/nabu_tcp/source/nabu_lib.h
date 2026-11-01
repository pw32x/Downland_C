// nabu_lib - a conversion of the SG-1000/Colecovision support of 
// devkitSMS (github.com/sverx/devkitSMS) to support the NABU computer.

#ifndef NABU_LIB_H
#define NABU_LIB_H

void NABU_init(void);

void NABU_VDPturnOnFeature(unsigned int feature);
void NABU_VDPturnOffFeature(unsigned int feature);
#define NABU_VDPFEATURE_SHOWDISPLAY      0x0140
#define NABU_VDPFEATURE_FRAMEIRQ         0x0120
#define NABU_VDPFEATURE_USELARGESPRITES  0x0102
#define NABU_VDPFEATURE_ZOOMSPRITES      0x0101

#define NABU_displayOn()   NABU_VDPturnOnFeature(NABU_VDPFEATURE_SHOWDISPLAY)
#define NABU_displayOff()  NABU_VDPturnOffFeature(NABU_VDPFEATURE_SHOWDISPLAY)

void NABU_setSpriteMode(unsigned char mode);
#define NABU_SPRITEMODE_NORMAL           0x00
#define NABU_SPRITEMODE_LARGE            0x01
#define NABU_SPRITEMODE_ZOOMED           0x02
#define NABU_SPRITEMODE_LARGE_ZOOMED     0x03

void NABU_setBackdropColor(unsigned char entry);
#define NABU_COLOR_TRANSPARENT           0x00
#define NABU_COLOR_BLACK                 0x01
#define NABU_COLOR_MEDIUM_GREEN          0x02
#define NABU_COLOR_LIGHT_GREEN           0x03
#define NABU_COLOR_DARK_BLUE             0x04
#define NABU_COLOR_LIGHT_BLUE            0x05
#define NABU_COLOR_DARK_RED              0x06
#define NABU_COLOR_CYAN                  0x07
#define NABU_COLOR_MEDIUM_RED            0x08
#define NABU_COLOR_LIGHT_RED             0x09
#define NABU_COLOR_DARK_YELLOW           0x0A
#define NABU_COLOR_LIGHT_YELLOW          0x0B
#define NABU_COLOR_DARK_GREEN            0x0C
#define NABU_COLOR_MAGENTA               0x0D
#define NABU_COLOR_GRAY                  0x0E
#define NABU_COLOR_WHITE                 0x0F

void NABU_waitForVBlank(void);

// Copy data into VRAM
// Note: tilefrom is a tile number, and size is in bytes
void NABU_loadTilePatterns(void *src, unsigned int tilefrom, unsigned int size);
void NABU_loadTileColours(void *src, unsigned int tilefrom, unsigned int size);
void NABU_loadSpritePatterns(void *src, unsigned int tilefrom, unsigned int size);

void NABU_loadTileMap(unsigned char x, unsigned char y, void *src, unsigned int size);
void NABU_loadTileMapArea(unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height);
void NABU_setNextTileatXY(unsigned char x, unsigned char y);
void NABU_setTile(unsigned char tile);
void NABU_getNextTileatXY(unsigned char x, unsigned char y);
unsigned char NABU_getTile(void);
#define NABU_setTileatXY(x,y,tile) do{NABU_setNextTileatXY((x),(y));NABU_setTile(tile);}while(0)
#define NABU_getTileatXY(x,y)      (NABU_getNextTileatXY((x),(y)),NABU_getTile())

// use order per frame:
// 1. NABU_initSprites
// 2. NABU_addSprite for all your sprites
// 3. NABU_finalizeSprites, 
// 4. NABU_copySpritestoSAT
void NABU_initSprites(void);
unsigned char NABU_addSprite(unsigned char x, unsigned char y, unsigned char tile, unsigned char attr); // 0 if no more sprites
void NABU_finalizeSprites(void);
void NABU_copySpritestoSAT(void);

#define PORT_A_KEY_UP     0x0001
#define PORT_A_KEY_DOWN   0x0002
#define PORT_A_KEY_LEFT   0x0004
#define PORT_A_KEY_RIGHT  0x0008
#define PORT_A_KEY_1      0x0010
#define PORT_A_KEY_2      0x0020
#define PORT_B_KEY_UP     0x0040
#define PORT_B_KEY_DOWN   0x0080
#define PORT_B_KEY_LEFT   0x0100
#define PORT_B_KEY_RIGHT  0x0200
#define PORT_B_KEY_1      0x0400
#define PORT_B_KEY_2      0x0800

// readJoysticks converts the NABU joystick values into KeysStatus
// in the SG-1000 version. 
extern volatile unsigned int KeysStatus;
void readJoysticks(void);

void NABU_VRAMmemcpy(unsigned int dst, void *src, unsigned int size);
void NABU_VRAMmemcpy_brief(unsigned int dst, void *src, unsigned char size);
void NABU_VRAMmemset(unsigned int dst, unsigned char value, unsigned int size);
void NABU_VRAMmemsetW(unsigned int dst, unsigned int value, unsigned int size);

void NABU_setFrameInterruptHandler(void (*theHandlerFunction)(void));

#endif
