#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <time.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

#define FB_WIDTH  256
#define FB_HEIGHT 192
#define FB_PITCH  (FB_WIDTH / 8)

#define SCREEN_SCALE 2
#define SCREEN_WIDTH (FB_WIDTH * SCREEN_SCALE)
#define SCREEN_HEIGHT (FB_HEIGHT * SCREEN_SCALE)

Uint8 framebuffer[FB_HEIGHT][FB_PITCH]; // 1-bit framebuffer
SDL_Texture* framebufferTexture = NULL;

// Set or clear a pixel in the 1-bit framebuffer
void set_pixel(int x, int y, int value) 
{
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) 
        return;

    Uint8 pixel = 1 << (7 - (x % 8));

    if (value)
        framebuffer[y][x / 8] |= pixel;  // Set bit (white pixel)
    else
        framebuffer[y][x / 8] &= ~pixel; // Clear bit (black pixel)
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
            Uint8 bit = (framebuffer[y][x / 8] >> (7 - (x % 8))) & 1;
            tex_pixels[y * (pitch / 4) + x] = bit ? 0xFFFFFFFF : 0xFF000000; // White or Black
        }
    }

    SDL_UnlockTexture(framebufferTexture);
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

    framebufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, FB_WIDTH, FB_HEIGHT);
    SDL_SetTextureScaleMode(framebufferTexture, SDL_SCALEMODE_NEAREST); // no smoothing

    srand(time(NULL));

    // clear framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));

    // add random pixels
    for (int i = 0; i < 500; i++) 
    {
        set_pixel(rand() % FB_WIDTH, rand() % FB_HEIGHT, 1);
    }

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
    // Update texture from framebuffer
    updateFramebufferTexture(framebufferTexture);

    // Render to screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, framebufferTexture, NULL, NULL); // Scale to window

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