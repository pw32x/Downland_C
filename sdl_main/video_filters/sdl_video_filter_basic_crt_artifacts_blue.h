#ifndef SDL_VIDEO_FILTER_BASIC_CRT_ARTIFACTS_BLUE
#define SDL_VIDEO_FILTER_BASIC_CRT_ARTIFACTS_BLUE

#include "sdl_video_filter_base.h"

extern "C"
{
#include "..\..\game\base_defines.h"
}

class SDLVideoFilterBasicCrtArtifactsBlue : public SDLVideoFilterBase
{
public:
	SDLVideoFilterBasicCrtArtifactsBlue(SDL_Renderer* renderer) : SDLVideoFilterBase(renderer) {}

	bool init() override;
	void shutdown() override;
	void update(unsigned char* gameFramebuffer) override;

private:
	unsigned int m_crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
};

#endif