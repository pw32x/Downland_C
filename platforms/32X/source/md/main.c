#include <stdint.h>
#include <string.h>

#include "../common/comm_ids.h"
#include "megasd.h"
#include "cd_init.h"
#include "vdp.h"

#include "data/genesis_palette.h"
#include "data/genesis_tileset.h"
#include "data/genesis_tilemap.h"
#include "data/32x_palette.h"
#include "data/32x_tileset.h"
#include "data/32x_tilemap.h"
#include "data/sphere32x32_4bpp.h"
#include "data/sphere32x32_4bpp_palette.h"
#include "data/font.h"

extern void init_main();
extern void bump_fm();
extern void enable_ints();
extern void disable_ints();
extern void chk_hotplug();
extern void update_fm();

// Communication channels                                               32X SIDE
static volatile uint16_t* const MD_SYS_COMM0 = (uint16_t*) 0xA15120; // 0x20004020
static volatile uint16_t* const MD_SYS_COMM2 = (uint16_t*) 0xA15122; // 0x20004022

void print_text(const char* str, int x, int y)
{
    while (*str)
        put_tile_xy(*str++, x++, y);
}

uint16_t crsr_y;
uint16_t crsr_x;

void process_commands()
{
    if (!*MD_SYS_COMM0)
        return;

    uint16_t command = *MD_SYS_COMM0 & 0xff00;
    uint16_t command_value = *MD_SYS_COMM0 & 0x00ff;

    /*
    if (command)
    {
        char buf[100];
        sprintf(buf, "%04X", command);
        print_text(buf, messageX, messageY);
        messageY++;

        if (messageY == 28)
        {
            messageY = 0;
            messageX += 5;
        }

        if (messageX > 40)
        {
            while (1) {};
        }
    }
    */



    if (command == SH2MD_COMMAND_SET_CURSOR)
    {
        uint16_t data = *MD_SYS_COMM2;
    
        crsr_y = (data & 0x1F);
        crsr_x = (data >> 6);
    }
    else if (command == SH2MD_COMMAND_PUT_CHAR)
    {
        uint16_t character = command_value;       // Extract the character
    
        put_tile_xy(character, crsr_x, crsr_y);
    
        crsr_x += 1;                             // Increment x cursor coordinate
    }
}

int sphere32x32_4bpp_tile_start;

void init_graphics()
{
    vpd_load_palette(genesis_palette, 1);
    int genesis_tileset_start = vdp_load_tileset(genesis_tileset, sizeof(genesis_tileset), sizeof(font));
    vdp_draw_tileset(genesis_tilemap, 0, 0, 40, 28, genesis_tileset_start, 1, PLANE_A_ADDR);

    vpd_load_palette(_32x_palette, 2);
    int _32x_tileset_start = vdp_load_tileset(_32x_tileset, sizeof(_32x_tileset), genesis_tileset_start + sizeof(genesis_tileset));
    vdp_draw_tileset(_32x_tilemap, 0, 0, 40, 28, _32x_tileset_start, 2, PLANE_B_ADDR);

    vpd_load_palette(sphere32x32_4bpp_palette, 3);
    int sphere32x32_4bpp_start = vdp_load_tileset((uint8_t*)sphere32x32_4bpp, sizeof(sphere32x32_4bpp), _32x_tileset_start + sizeof(_32x_tileset));
    sphere32x32_4bpp_tile_start = sphere32x32_4bpp_start / 32;
}

int planeAScrollX = 0;
int planeAScrollY = 0;
int planeBScrollX = 0;
int planeBScrollY = 0;

int planeAScrollXDirection = 1;
int planeAScrollYDirection = 1;
int planeBScrollXDirection = -1;
int planeBScrollYDirection = -1;


#define SPRITE_WIDTH 32
#define SPRITE_HEIGHT 32

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t direction_x;
    int16_t direction_y;
} sprite;

sprite sprites[HARDWARE_SPRITE_LIMIT];

int16_t random_values[40] = 
{
    3, -1,
    -2, 3,
    1, 1,
    -2, -1,
    1, -2,
    -2, -3,
    1, -2,
    2, 2,
    3, 2,
    -3, -2,
    -1, -2,
    -2, 2,
    3, -3,
    2, 1,
    -2, -3,
    -1, -1,
    -1, 2,
    -1, 3,
    1, 3,
    -3, -2
};

void init_scene()
{
    sprite* current_sprite = sprites;

    for (int loopy = 0; loopy < 8; loopy++)
    {
        for (int loopx = 0; loopx < 10; loopx++)
        {
            current_sprite->x = (loopx * 32);
            current_sprite->y = (loopy * 24);
            
            current_sprite->direction_x = random_values[(loopy + loopx) % 40] ;
            current_sprite->direction_y = random_values[(loopy + loopx + 1) % 40];

            current_sprite++;
        }
    }
}

void update_scene()
{
    planeBScrollX += planeBScrollXDirection;
    if (planeBScrollX > 100 || planeBScrollX < 0)
        planeBScrollXDirection = -planeBScrollXDirection;

    planeAScrollY += planeAScrollYDirection;
    if (planeAScrollY > 100 || planeAScrollY < 0)
        planeAScrollYDirection = -planeAScrollYDirection;

        vdp_set_vertical_scroll(PLANE_A_ADDR, planeAScrollY);
        vdp_set_horizontal_scroll(PLANE_B_ADDR, planeBScrollX);
 
    for (int loop = 0; loop < HARDWARE_SPRITE_LIMIT; loop++)
    {
        sprite* current_sprite = sprites + loop;

        current_sprite->x += current_sprite->direction_x;
        current_sprite->y += current_sprite->direction_y;

        if (current_sprite->x < 0 || current_sprite->x + SPRITE_WIDTH > SCREEN_WIDTH)
            current_sprite->direction_x = -current_sprite->direction_x;

        if (current_sprite->y < 0 || current_sprite->y + SPRITE_HEIGHT > SCREEN_HEIGHT)
            current_sprite->direction_y = -current_sprite->direction_y;

        vdp_push_hardware_sprite(current_sprite->x, current_sprite->y, SPRITE_SIZE(4, 4), sphere32x32_4bpp_tile_start | (3 << 13));
    }


}

int main(void)
{
    cd_ok = InitCD();
    megasd_ok = InitMegaSD();

    init_main();
    init_graphics();
    init_scene();

    print_text("Hello World from MD side", 0, 2);

    while (1)
    {
        // disabling and enabling ints screws up the MD_SYS_COMM0 messages in process_commands
        // for some reason. it doesn't occur in assembly.
        //disable_ints();
        //bump_fm();
        //enable_ints();

        //update_fm();

        process_commands();
        *MD_SYS_COMM0 = 0;

        chk_hotplug();

        update_scene();
        

        vdp_wait_vsync();
        vdp_upload_hardware_sprites();
    }

    return 0;
}
