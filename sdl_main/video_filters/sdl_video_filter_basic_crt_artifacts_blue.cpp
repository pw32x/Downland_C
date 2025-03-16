#include "sdl_video_filter_basic_crt_artifacts_blue.h"
#include "sdl_video_filter_utils.h"

bool SDLVideoFilterBasicCrtArtifactsBlue::init()
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

void SDLVideoFilterBasicCrtArtifactsBlue::shutdown()
{
    SDLVideoFilterBase::shutdown();
}

void SDLVideoFilterBasicCrtArtifactsBlue::update(unsigned char* gameFramebuffer)
{
    if (m_outputTexture == nullptr)
        init();

    // Update texture from gameFramebuffer
    SDLUtils_updateCrtFramebufferAndTexture(gameFramebuffer,
                                            m_crtFramebuffer,
                                            m_outputTexture,
                                            CrtColor::Blue,
                                            m_renderer);
}
