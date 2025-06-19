#ifndef SDL_VIDEO_FILTER_BASIC_CRT_ARTIFACTS_ORANGE
#define SDL_VIDEO_FILTER_BASIC_CRT_ARTIFACTS_ORANGE

#include "sdl_video_filter_base.h"
#include "..\..\..\game\base_defines.h"

class SDLVideoFilterBasicCrtArtifactsOrange : public SDLVideoFilterBase
{
public:
	SDLVideoFilterBasicCrtArtifactsOrange(SDL_Renderer* renderer, 
										  const Resources* resources) 
		: SDLVideoFilterBase(renderer, resources) {}

	bool init() override;
	void shutdown() override;
	void update(const GameData* gameData) override;

private:
	unsigned int m_crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects
};

#endif