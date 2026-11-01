#include <arch/nabu.h>
#include <interrupt.h>
#include <games.h>

#include "nabu_lib.h"

#define MAXSPRITES 32

// VRAM map (the VDP Mode 2 layout)
#define PNTADDRESS 0x1800   // pattern name table for tile maps
#define SATADDRESS 0x1B00   // sprite attribute table
#define PGTADDRESS 0x0000   // pattern generator table for tiles
#define CGTADDRESS 0x2000   // per-line colour table for tiles
#define SGTADDRESS 0x3800   // sprite generator table

#define LO(x) ((unsigned char)((x) & 0xff))
#define HI(x) ((unsigned char)(((x) >> 8) & 0xff))

// the VDP registers at start: Mode 2, 16K, display off, frame interrupt on, 8x8 sprites, black backdrop
static const unsigned char VDPReg_init[8] = { 0x02, 0xa0, 0x06, 0xff, 0x03, 0x36, 0x07, 0x01 };

// shadow of registers 0 and 1, which hold the feature bits
static unsigned char VDPReg[2] = { 0x02, 0xa0 };

static volatile unsigned char VDPBlank;

unsigned char SpriteTable[MAXSPRITES * 4];
unsigned char SpriteNextFree;

static void (*NABU_theFrameInterruptHandler)(void);

static void write_to_VDPRegister(unsigned char reg, unsigned char value)
{
    IO_VDPLATCH = value;
    IO_VDPLATCH = 0x80 | reg;
}

static void set_address_VRAM(unsigned int address)
{
    IO_VDPLATCH = LO(address);
    IO_VDPLATCH = HI(address) | 0x40;
}

static void set_address_VRAM_read(unsigned int address)
{
    IO_VDPLATCH = LO(address);
    IO_VDPLATCH = HI(address);
}

static void byte_array_to_VDP_data(const unsigned char *data, unsigned int size)
{
    while (size--)
    {
        IO_VDPDATA = *data++;
    }
}

void nabulib_frame(void)
{
    VDPBlank = 1;

    if (NABU_theFrameInterruptHandler)
    {
        NABU_theFrameInterruptHandler();
    }
}

static void nabulib_isr(void) __naked
{
    __asm
    push    ix
    push    iy
    call    _nabulib_frame
    pop     iy
    pop     ix
    ret
    __endasm;
}

void NABU_VDPturnOnFeature(unsigned int feature)
{
    VDPReg[HI(feature)] |= LO(feature);
    write_to_VDPRegister(HI(feature), VDPReg[HI(feature)]);
}

void NABU_VDPturnOffFeature(unsigned int feature)
{
    VDPReg[HI(feature)] &= ~LO(feature);
    write_to_VDPRegister(HI(feature), VDPReg[HI(feature)]);
}

void NABU_init(void)
{
    unsigned char i;

    for (i = 0; i < 8; i++)
    {
        write_to_VDPRegister(i, VDPReg_init[i]);
    }
    VDPReg[0] = VDPReg_init[0];
    VDPReg[1] = VDPReg_init[1];

    NABU_initSprites();
    NABU_finalizeSprites();
    NABU_copySpritestoSAT();

    im1_install_isr(nabulib_isr);
}

void NABU_setBackdropColor(unsigned char entry)
{
    write_to_VDPRegister(0x07, entry);
}

void NABU_setSpriteMode(unsigned char mode)
{
    if (mode & NABU_SPRITEMODE_LARGE)
    {
        NABU_VDPturnOnFeature(NABU_VDPFEATURE_USELARGESPRITES);
    }
    else
    {
        NABU_VDPturnOffFeature(NABU_VDPFEATURE_USELARGESPRITES);
    }

    if (mode & NABU_SPRITEMODE_ZOOMED)
    {
        NABU_VDPturnOnFeature(NABU_VDPFEATURE_ZOOMSPRITES);
    }
    else
    {
        NABU_VDPturnOffFeature(NABU_VDPFEATURE_ZOOMSPRITES);
    }
}

void NABU_loadTilePatterns(void *src, unsigned int tilefrom, unsigned int size)
{
    set_address_VRAM(PGTADDRESS + (tilefrom << 3));
    byte_array_to_VDP_data(src, size);
}

void NABU_loadTileColours(void *src, unsigned int tilefrom, unsigned int size)
{
    set_address_VRAM(CGTADDRESS + (tilefrom << 3));
    byte_array_to_VDP_data(src, size);
}

void NABU_loadSpritePatterns(void *src, unsigned int tilefrom, unsigned int size)
{
    set_address_VRAM(SGTADDRESS + (tilefrom << 3));
    byte_array_to_VDP_data(src, size);
}

void NABU_setNextTileatXY(unsigned char x, unsigned char y)
{
    set_address_VRAM(PNTADDRESS + ((unsigned int)y << 5) + x);
}

void NABU_setTile(unsigned char tile)
{
    IO_VDPDATA = tile;
}

void NABU_getNextTileatXY(unsigned char x, unsigned char y)
{
    set_address_VRAM_read(PNTADDRESS + ((unsigned int)y << 5) + x);
}

unsigned char NABU_getTile(void)
{
    return IO_VDPDATA;
}

void NABU_loadTileMap(unsigned char x, unsigned char y, void *src, unsigned int size)
{
    set_address_VRAM(PNTADDRESS + ((unsigned int)y << 5) + x);
    byte_array_to_VDP_data(src, size);
}

void NABU_loadTileMapArea(unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height)
{
    unsigned char cur_y;

    for (cur_y = y; cur_y < (unsigned char)(y + height); cur_y++)
    {
        set_address_VRAM(PNTADDRESS + ((unsigned int)cur_y << 5) + x);
        byte_array_to_VDP_data(src, width);
        src = (unsigned char *)src + width;
    }
}

void NABU_initSprites(void)
{
    SpriteNextFree = 0;
}

unsigned char NABU_addSprite(unsigned char x, unsigned char y, unsigned char tile, unsigned char attr)
{
    unsigned char idx;

    if (SpriteNextFree < MAXSPRITES)
    {
        if ((unsigned char)(y - 1) != 0xd0)
        {
            idx = SpriteNextFree << 2;
            SpriteTable[idx++] = y;
            SpriteTable[idx++] = x;
            SpriteTable[idx++] = tile;
            SpriteTable[idx] = attr;
            SpriteNextFree++;
        }
        return 1;
    }
    return 0;
}

void NABU_finalizeSprites(void)
{
    if (SpriteNextFree < MAXSPRITES)
    {
        SpriteTable[SpriteNextFree << 2] = 0xd0;
    }
}

void NABU_copySpritestoSAT(void)
{
    set_address_VRAM(SATADDRESS);
    byte_array_to_VDP_data(SpriteTable, MAXSPRITES * 4);
}

void NABU_waitForVBlank(void)
{
    VDPBlank = 0;
    while (!VDPBlank)
    {
    }
}

void NABU_VRAMmemcpy(unsigned int dst, void *src, unsigned int size)
{
    set_address_VRAM(dst);
    byte_array_to_VDP_data(src, size);
}

void NABU_VRAMmemcpy_brief(unsigned int dst, void *src, unsigned char size)
{
    set_address_VRAM(dst);
    byte_array_to_VDP_data(src, size);
}

void NABU_VRAMmemset(unsigned int dst, unsigned char value, unsigned int size)
{
    set_address_VRAM(dst);
    while (size--)
    {
        IO_VDPDATA = value;
    }
}

void NABU_VRAMmemsetW(unsigned int dst, unsigned int value, unsigned int size)
{
    set_address_VRAM(dst);
    while (size > 1)
    {
        IO_VDPDATA = LO(value);
        IO_VDPDATA = HI(value);
        size -= 2;
    }
}

void NABU_setFrameInterruptHandler(void (*theHandlerFunction)(void))
{
    NABU_theFrameInterruptHandler = theHandlerFunction;
}

volatile unsigned int KeysStatus;

void readJoysticks(void)
{
	unsigned int a = joystick(1);
	unsigned int b = joystick(2);
	unsigned int keys = 0;

	if (a & MOVE_UP)    keys |= PORT_A_KEY_UP;
	if (a & MOVE_DOWN)  keys |= PORT_A_KEY_DOWN;
	if (a & MOVE_LEFT)  keys |= PORT_A_KEY_LEFT;
	if (a & MOVE_RIGHT) keys |= PORT_A_KEY_RIGHT;
	if (a & MOVE_FIRE)  keys |= PORT_A_KEY_1;

	if (b & MOVE_UP)    keys |= PORT_B_KEY_UP;
	if (b & MOVE_DOWN)  keys |= PORT_B_KEY_DOWN;
	if (b & MOVE_LEFT)  keys |= PORT_B_KEY_LEFT;
	if (b & MOVE_RIGHT) keys |= PORT_B_KEY_RIGHT;
	if (b & MOVE_FIRE)  keys |= PORT_B_KEY_1;

	KeysStatus = keys;
}
