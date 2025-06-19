#ifndef SDL_VIDEO_FILTER_BASIC_CRT_ARTIFACTS_BLUE
#define SDL_VIDEO_FILTER_BASIC_CRT_ARTIFACTS_BLUE

#include "sdl_video_filter_base.h"

extern "C"
{
#include "..\..\..\game\base_defines.h"
}

class SDLVideoFilterBasicCrtArtifactsBlue : public SDLVideoFilterBase
{
public:
	SDLVideoFilterBasicCrtArtifactsBlue(SDL_Renderer* renderer, 
										const Resources* resources) 
		: SDLVideoFilterBase(renderer, resources) {}

	bool init() override;
	void shutdown() override;
	void update(const GameData* gameData) override;

private:
	unsigned int m_crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
};

#endif