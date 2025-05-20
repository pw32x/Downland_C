#ifndef SDL_VIDEO_FILTER_MODERN
#define SDL_VIDEO_FILTER_MODERN

#include "..\sdl_video_filter_base.h"
#include <vector>

// This is an example of using the game state to render
// a game frame, instead of using the raw simulated
// CoCo framebuffer. With this, we can start looking into 
// plugging in different kinds of renderers.

extern "C"
{
#include "..\..\..\game\base_defines.h"
}

typedef std::vector<u32> SpriteFrame;

class Sprite
{
public:
	Sprite(const u8* orginalSprite, u8 width, u8 height, u8 numFrames);
public:

	u8 m_width;
	u8 m_height;
	u8 m_numFrames;
	std::vector<SpriteFrame> m_frames;
};

class SDLVideoFilterModern : public SDLVideoFilterBase
{
public:
	SDLVideoFilterModern(SDL_Renderer* renderer, 
						 const Resources* resources);

	bool init() override;
	void shutdown() override;
	void update(const GameData* gameData) override;

private:
	unsigned int m_crtFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT]; // frame buffer for basic CRT artifact effects

	// we build 32bit sprites from the raw 1bpp sprites
	Sprite m_dropSprite;
	Sprite m_ballSprite;
	Sprite m_playerSprite;
	Sprite m_playerSplatSprite;
	Sprite m_birdSprite;
	Sprite m_keySprite;
	Sprite m_diamondSprite;
	Sprite m_moneyBagSprite;

	std::vector<const Sprite*> m_pickUpSprites;
};

#endif