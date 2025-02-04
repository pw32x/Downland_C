#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <time.h>

#include "screenshot_data.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#define FB_WIDTH  256
#define FB_HEIGHT 192
#define FB_PITCH  (FB_WIDTH / 8)

#define SCREEN_SCALE 3
#define SCREEN_WIDTH (FB_WIDTH * SCREEN_SCALE)
#define SCREEN_HEIGHT (FB_HEIGHT * SCREEN_SCALE)

Uint8 framebuffer[FB_HEIGHT * FB_PITCH]; // 1-bit framebuffer
SDL_Texture* framebufferTexture = NULL;

// half frame buffer where we will generate CRT artifact effects
uint32_t crtFramebuffer[FB_WIDTH * FB_HEIGHT];
SDL_Texture* crtFramebufferTexture = NULL;

// Set or clear a pixel in the 1-bit framebuffer
void set_pixel(int x, int y, int value) 
{
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) 
        return;

    Uint8 pixel = 1 << (7 - (x % 8));
    int index = (x / 8) + (y * FB_PITCH);

    if (value)
        framebuffer[index] |= pixel;  // Set bit (white pixel)
    else
        framebuffer[index] &= ~pixel; // Clear bit (black pixel)
}

// Convert the 1-bit framebuffer into a texture
void updateFramebufferTexture(SDL_Texture* framebufferTexture) 
{
    void* pixels;
    int pitch;
    
    // Lock texture to modify pixel data
    SDL_LockTexture(framebufferTexture, NULL, &pixels, &pitch);
    Uint32* tex_pixels = (Uint32*)pixels;
    
    // Convert 1-bit buffer to 32-bit texture pixels
    for (int y = 0; y < FB_HEIGHT; y++) 
    {
        for (int x = 0; x < FB_WIDTH; x++) 
        {
            Uint8 bit = (framebuffer[(x / 8) + (y * FB_PITCH)] >> (7 - (x % 8))) & 1;
            tex_pixels[y * (pitch / 4) + x] = bit ? 0xFFFFFFFF : 0xFF000000; // White or Black
        }
    }

    SDL_UnlockTexture(framebufferTexture);
}


void updateCrtFramebufferAndTexture(SDL_Renderer* renderer) 
{
    // Color definitions
    const uint32_t BLACK  = 0x000000; // 00 black
    const uint32_t BLUE   = 0x0000FF; // 01 blue
    const uint32_t ORANGE = 0xFFA500; // 10 orange
    const uint32_t WHITE  = 0xFFFFFF; // 11 white

    for (int y = 0; y < FB_HEIGHT; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.

        for (int x = 0; x < FB_WIDTH; x += 2) 
        {
            int byteIndex = (y * 32) + (x / 8);
            int bitOffset = 7 - (x % 8);

            // Read two adjacent bits
            uint8_t bit1 = (framebuffer[byteIndex] >> bitOffset) & 1;
            uint8_t bit2 = (framebuffer[byteIndex] >> (bitOffset - 1)) & 1;

            // Determine base color
            uint32_t color = BLACK;
            if (bit1 == 0 && bit2 == 1) color = BLUE;
            else if (bit1 == 1 && bit2 == 0) color = ORANGE;
            else if (bit1 == 1 && bit2 == 1) color = WHITE;

            // Apply base color
            crtFramebuffer[y * FB_WIDTH + x]     = color;
            crtFramebuffer[y * FB_WIDTH + x + 1] = color;
        }

        // Apply a quick and dirty crt artifact effect
        // pixels whose original bits are adjacent are converted to white
        // source colors: black, black, blue, blue, orange, orange, white, white.
        // source bits:  00 01 10 11
        // seen as:      00 00 01 01 10 10 11 11 // forth and fifth pairs have adjacent bits. 
        //                                          Turn both corresponding pixels to white.
        //                                          Also turn off the other pixel in the pair to
        //                                          black.
        // final final:  black, black, black, white, white, black, white, white
        for (int x = 0; x < FB_WIDTH; x += 2) 
        {
            uint32_t pixel1 = crtFramebuffer[y * FB_WIDTH + x];
            uint32_t pixel2 = crtFramebuffer[y * FB_WIDTH + x + 1];

            if (pixel2 == BLUE && x < FB_WIDTH - 2)
            {
                uint32_t pixel3 = crtFramebuffer[y * FB_WIDTH + x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    pixel2 = WHITE;
                    pixel1 = BLACK;
                }
            }
            else if (pixel1 == ORANGE && x > 2)
            {
                uint32_t pixel0 = crtFramebuffer[y * FB_WIDTH + x - 1];
                if (pixel0 == BLUE || pixel0 == WHITE)
                {
                    pixel1 = WHITE;
                    pixel2 = BLACK;
                }
            }

            crtFramebuffer[y * FB_WIDTH + x] = pixel1;
            crtFramebuffer[y * FB_WIDTH + x + 1] = pixel2;
        }
    }

    // Update the texture with the new data
    SDL_UpdateTexture(crtFramebufferTexture, NULL, crtFramebuffer, FB_WIDTH * sizeof(uint32_t));
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Downland_C", "1.0", "com.example.Downland_C");

    if (!SDL_Init(SDL_INIT_VIDEO)) 
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Downland_C", 
                                     SCREEN_WIDTH, 
                                     SCREEN_HEIGHT, 
                                     0, 
                                     &window, 
                                     &renderer)) 
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    framebufferTexture = SDL_CreateTexture(renderer, 
                                           SDL_PIXELFORMAT_XRGB8888, 
                                           SDL_TEXTUREACCESS_STREAMING, 
                                           FB_WIDTH, FB_HEIGHT);
    SDL_SetTextureScaleMode(framebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing

    // Create the texture
    crtFramebufferTexture = SDL_CreateTexture(renderer, 
                                               SDL_PIXELFORMAT_XRGB8888, 
                                               SDL_TEXTUREACCESS_STREAMING, 
                                               FB_WIDTH, 
                                               FB_HEIGHT);
    SDL_SetTextureScaleMode(crtFramebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing

    srand((unsigned int)time(NULL));

    memcpy(framebuffer, screenshot_data, 0x1800);

    /*
    // clear framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));


    // add random pixels
    for (int i = 0; i < 500; i++) 
    {
        set_pixel(rand() % FB_WIDTH, rand() % FB_HEIGHT, 1);
    }
    */


    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    // Render to screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Update texture from framebuffer
    updateFramebufferTexture(framebufferTexture); 
    //SDL_RenderTexture(renderer, framebufferTexture, NULL, NULL); // Scale to window

    // Update texture from crtFramebuffer
    updateCrtFramebufferAndTexture(renderer);
    SDL_RenderTexture(renderer, crtFramebufferTexture, NULL, NULL); // Scale to window

    /*
    // write debug text
    SDL_SetRenderScale(renderer, 1.5f, 1.5f);
    SDL_SetRenderDrawColor(renderer, 51, 102, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, 10.0f, 10.0f, "Some debug text");
    SDL_RenderDebugTextFormat(renderer, 10.0f, 20.0f, "This is line number %d", 2);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    */

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */

    SDL_DestroyTexture(framebufferTexture);
}