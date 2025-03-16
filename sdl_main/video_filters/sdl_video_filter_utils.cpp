#include "sdl_video_filter_utils.h"

// Convert the 1-bit framebuffer into a texture
void SDLUtils_updateFramebufferTexture(u8* framebuffer, 
                                       SDL_Texture* framebufferTexture) 
{
    void* pixels;
    int pitch;
    
    // Lock texture to modify pixel data
    SDL_LockTexture(framebufferTexture, NULL, &pixels, &pitch);
    Uint32* tex_pixels = (Uint32*)pixels;
    
    // Convert 1-bit buffer to 32-bit texture pixels
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) 
    {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) 
        {
            Uint8 bit = (framebuffer[(x / 8) + (y * FRAMEBUFFER_PITCH)] >> (7 - (x % 8))) & 1;
            tex_pixels[y * (pitch / 4) + x] = bit ? 0xFFFFFFFF : 0xFF000000; // White or Black
        }
    }

    SDL_UnlockTexture(framebufferTexture);
}


void SDLUtils_updateCrtFramebufferAndTexture(u8* framebuffer,
                                             u32* crtFramebuffer,
                                             SDL_Texture* crtFramebufferTexture,
                                             CrtColor crtColor,
                                             SDL_Renderer* renderer) 
{
    // Color definitions
    const uint32_t BLACK  = 0x000000; // 00 black
    const uint32_t BLUE   = crtColor == CrtColor::Blue ? 0x0000FF : 0xFFA500; // 01 blue
    const uint32_t ORANGE = crtColor == CrtColor::Blue ? 0xFFA500 : 0x0000FF; // 10 orange
    const uint32_t WHITE  = 0xFFFFFF; // 11 white

    for (int y = 0; y < FRAMEBUFFER_HEIGHT; ++y) 
    {
        // every pair of bits generates a color for the two corresponding
        // pixels of the destination texture, so:
        // source bits:        00 01 10 11
        // final pixel colors: black, black, blue, blue, orange, orange, white, white.

        for (int x = 0; x < FRAMEBUFFER_WIDTH; x += 2) 
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
            crtFramebuffer[y * FRAMEBUFFER_WIDTH + x]     = color;
            crtFramebuffer[y * FRAMEBUFFER_WIDTH + x + 1] = color;
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
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x += 2) 
        {
            uint32_t pixel1 = crtFramebuffer[y * FRAMEBUFFER_WIDTH + x];
            uint32_t pixel2 = crtFramebuffer[y * FRAMEBUFFER_WIDTH + x + 1];

            if (pixel2 == BLUE && x < FRAMEBUFFER_WIDTH - 2)
            {
                uint32_t pixel3 = crtFramebuffer[y * FRAMEBUFFER_WIDTH + x + 2];
                if (pixel3 == ORANGE || pixel3 == WHITE)
                {
                    pixel2 = WHITE;
                    pixel1 = BLACK;
                }
            }
            else if (pixel1 == ORANGE && x > 2)
            {
                uint32_t pixel0 = crtFramebuffer[y * FRAMEBUFFER_WIDTH + x - 1];
                if (pixel0 == BLUE || pixel0 == WHITE)
                {
                    pixel1 = WHITE;
                    pixel2 = BLACK;
                }
            }

            crtFramebuffer[y * FRAMEBUFFER_WIDTH + x] = pixel1;
            crtFramebuffer[y * FRAMEBUFFER_WIDTH + x + 1] = pixel2;
        }
    }

    // Update the texture with the new data
    SDL_UpdateTexture(crtFramebufferTexture, NULL, crtFramebuffer, FRAMEBUFFER_WIDTH * sizeof(uint32_t));
}

void SDLUtils_updateDebugFramebufferTexture(u32* debugFramebuffer, 
                                            SDL_Texture* debugFramebufferTexture)
{
    void* pixels;
    int pitch;

    // Lock texture to modify pixel data
    SDL_LockTexture(debugFramebufferTexture, NULL, &pixels, &pitch);

    Uint32* tex_pixels = (Uint32*)pixels;

    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) 
    {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) 
        {
            tex_pixels[(y * (pitch / 4)) + x] = debugFramebuffer[x + (y * FRAMEBUFFER_WIDTH)];
        }
    }

    SDL_UnlockTexture(debugFramebufferTexture);
}

void SDLUtils_computeDestinationRect(int screenWidth, 
                                     int screenHeight, 
                                     int framebufferWidth, 
                                     int framebufferHeight, 
                                     SDL_FRect* outRect) 
{
    float screenAspect = (float)screenWidth / screenHeight;
    float texAspect = (float)framebufferWidth / framebufferHeight;

    float scale;
    if (screenAspect > texAspect) 
    {
        // Screen is wider than the texture's aspect ratio, fit by height
        scale = (float)screenHeight / framebufferHeight;
    } 
    else 
    {
        // Screen is taller than the texture's aspect ratio, fit by width
        scale = (float)screenWidth / framebufferWidth;
    }

    // Compute final width and height
    outRect->w = framebufferWidth * scale;
    outRect->h = framebufferHeight * scale;

    // Center the rectangle on the screen
    outRect->x = (screenWidth - outRect->w) / 2;
    outRect->y = (screenHeight - outRect->h) / 2;
}