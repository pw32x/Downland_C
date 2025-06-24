#ifndef SDL_IMAGE_UTILS_INCLUDE_H
#define SDL_IMAGE_UTILS_INCLUDE_H

extern "C"
{
#include "../../game/base_defines.h"
#include "../../game/base_types.h"
}

#include <SDL3/SDL.h>


// Convert the 1-bit framebuffer into a texture
void SDLUtils_updateFramebufferTexture(const dl_u8* framebuffer, 
                                       SDL_Texture* framebufferTexture);


// take the framebuffer and apply basic CRT artifacts, updating a
// second framebuffer and a texture for it.
enum CrtColor
{
    Blue,
    Orange
};

void SDLUtils_convert1bppImageTo32bppCrtEffectImage(const dl_u8* originalImage,
                                                    dl_u32* crtImage,
                                                    dl_u16 width,
                                                    dl_u16 height,
                                                    CrtColor crtColor);

void SDLUtils_updateDebugFramebufferTexture(dl_u32* debugFramebuffer, 
                                            SDL_Texture* debugFramebufferTexture);

void SDLUtils_computeDestinationRect(int screenWidth, 
                                     int screenHeight, 
                                     int framebufferWidth, 
                                     int framebufferHeight, 
                                     SDL_FRect* outRect);

#endif