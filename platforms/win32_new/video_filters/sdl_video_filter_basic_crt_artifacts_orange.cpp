#include "sdl_video_filter_basic_crt_artifacts_orange.h"
#include "sdl_video_filter_utils.h"

bool SDLVideoFilterBasicCrtArtifactsOrange::init()
{
    // Create the texture
    m_outputTexture = SDL_CreateTexture(m_renderer, 
                                        SDL_PIXELFORMAT_XRGB8888, 
                                        SDL_TEXTUREACCESS_STREAMING, 
                                        FRAMEBUFFER_WIDTH, 
                                        FRAMEBUFFER_HEIGHT);

    SDL_SetTextureScaleMode(m_outputTexture, SDL_SCALEMODE_NEAREST); // no smoothing

	return true;
}

void SDLVideoFilterBasicCrtArtifactsOrange::shutdown()
{
    SDLVideoFilterBase::shutdown();
}

void SDLVideoFilterBasicCrtArtifactsOrange::update(const GameData* gameData)
{
    if (m_outputTexture == nullptr)
        init();

    // Update texture from gameFramebuffer
    SDLUtils_convert1bppImageTo32bppCrtEffectImage(gameData->framebuffer,
                                                   m_crtFramebuffer,
                                                   FRAMEBUFFER_WIDTH,
                                                   FRAMEBUFFER_HEIGHT,
                                                   CrtColor::Orange);


    // Update the texture with the new data
    SDL_UpdateTexture(m_outputTexture, 
                      NULL, 
                      m_crtFramebuffer, 
                      FRAMEBUFFER_WIDTH * sizeof(uint32_t));
}
