#ifndef SDL_VIDEO_FILTER_UTILS_INCLUDE_H
#define SDL_VIDEO_FILTER_UTILS_INCLUDE_H

extern "C"
{
#include "../../game/base_defines.h"
#include "../../game/base_types.h"
}

#include <SDL3/SDL.h>


// Convert the 1-bit framebuffer into a texture
void SDLUtils_updateFramebufferTexture(u8* framebuffer, 
                                       SDL_Texture* framebufferTexture);


// take the framebuffer and apply basic CRT artifacts, updating a
// second framebuffer and a texture for it.
enum CrtColor
{
    Blue,
    Orange
};

void SDLUtils_updateCrtFramebufferAndTexture(u8* framebuffer,
                                             u32* crtFramebuffer,
                                             SDL_Texture* crtFramebufferTexture,
                                             CrtColor crtColor,
                                             SDL_Renderer* renderer);

void SDLUtils_updateDebugFramebufferTexture(u32* debugFramebuffer, 
                                            SDL_Texture* debugFramebufferTexture);

#endif