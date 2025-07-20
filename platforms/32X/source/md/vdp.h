#ifndef VDP_INCLUDE_H
#define VDP_INCLUDE_H

#include <stdint.h>

#define PLANE_A_ADDR        0xC000
#define PLANE_W_ADDR        0xB000
#define PLANE_B_ADDR        0xE000
#define SPRITE_ATTR_ADDR    0xA800
#define HSCROLL_ADDR        0xAC00

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224

#define PLANE_WIDTH     64
#define PLANE_HEIGHT    32

// Helpful macros from SGDK https://github.com/Stephane-D/SGDK
#define VDP_VBLANK_FLAG         (1 << 3)

#define GFX_WRITE_CRAM_ADDR(adr)    (((0xC000 + ((adr) & 0x7F)) << 16) + 0x00)
#define GFX_WRITE_VRAM_ADDR(adr)    (((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00))
#define GFX_WRITE_VSRAM_ADDR(adr)   (((0x4000 + ((adr) & 0x3F)) << 16) + 0x10)

#define SPRITE_SIZE(w, h)   ((((w) - 1) << 2) | ((h) - 1))

extern volatile uint16_t* const vdp_data_port;
extern volatile uint32_t* const vdp_data_wide;
extern volatile uint16_t* const vdp_ctrl_port;
extern volatile uint32_t* const vdp_ctrl_wide;

extern uint32_t fg_color;
extern uint32_t bg_color;

void vdp_wait_vsync();

inline void vdp_set_autoinc(uint8_t value)
{
    *vdp_ctrl_port = 0x8F00 | value;
}

// Palettes
void vpd_load_palette(const uint16_t* palette, uint16_t paletteIndex);


// Tiles and planes
uint32_t vdp_load_tileset(const uint8_t* tileset, uint16_t tilesetSize, uint32_t vdpAddress);

void vdp_draw_tileset(const uint16_t* tilemap, 
                      int16_t x, 
                      int16_t y, 
                      uint16_t width, 
                      uint16_t height, 
                      uint16_t tilesetStart,
                      uint16_t paletteIndex,
                      uint16_t planeAddr);

void vdp_load_font();
void put_tile_xy(uint16_t tile, uint16_t x, uint16_t y);

void vdp_set_horizontal_scroll(uint16_t planeAddr, int16_t scrollAmount);
void vdp_set_vertical_scroll(uint16_t planeAddr, int16_t scrollAmount);

// Sprites
typedef struct
{
    int16_t y;
    uint8_t size;
    uint8_t link;
    uint16_t attributes;
    int16_t x;
}  hardware_sprite;

#define HARDWARE_SPRITE_LIMIT 80
extern hardware_sprite hardware_sprites[HARDWARE_SPRITE_LIMIT];
extern int hardware_sprite_count;
extern hardware_sprite* current_hardware_sprite;

void vdp_init_hardware_sprites();
void vdp_push_hardware_sprite(int16_t x, int16_t y, uint8_t size, uint16_t attributes);
void vdp_upload_hardware_sprites();



#endif