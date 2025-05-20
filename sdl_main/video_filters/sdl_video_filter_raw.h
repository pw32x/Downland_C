#ifndef SDL_VIDEO_FILTER_RAW
#define SDL_VIDEO_FILTER_RAW

#include "sdl_video_filter_base.h"

class SDLVideoFilterRaw : public SDLVideoFilterBase
{
public:
	SDLVideoFilterRaw(SDL_Renderer* renderer, 
					  const Resources* resources) 
		: SDLVideoFilterBase(renderer, resources) {}

	bool init() override;
	void shutdown() override;
	void update(const GameData* gameData) override;
};

#endif