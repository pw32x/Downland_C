#include "sdl_video_filter_raw.h"
#include "sdl_video_filter_utils.h"

#include "..\..\..\game\base_defines.h"

bool SDLVideoFilterRaw::init()
{
    m_outputTexture = SDL_CreateTexture(m_renderer, 
                                        SDL_PIXELFORMAT_XRGB8888, 
                                        SDL_TEXTUREACCESS_STREAMING, 
                                        FRAMEBUFFER_WIDTH, 
                                        FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(m_outputTexture, SDL_SCALEMODE_NEAREST); // no smoothing

	return true;
}

void SDLVideoFilterRaw::shutdown()
{
    SDLVideoFilterBase::shutdown();
}

void SDLVideoFilterRaw::update(const GameData* gameData)
{
    if (m_outputTexture == nullptr)
        init();

    // Update texture from framebuffer
    SDLUtils_updateFramebufferTexture(gameData->framebuffer, m_outputTexture); 
}
