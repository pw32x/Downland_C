#ifndef SDL_VIDEO_FILTER_NEW_RENDERER
#define SDL_VIDEO_FILTER_NEW_RENDERER

#include "sdl_video_filter_base.h"
#include <vector>

// This is an example of using the game state to render
// a game frame, instead of using the raw simulated
// CoCo framebuffer. With this, we can start looking into 
// plugging in different kinds of renderers.

extern "C"
{
#include "..\..\game\base_defines.h"
}

typedef std::vector<u32> SpriteFrame;

class Sprite
{
public:
	Sprite(const u8* orginalSprite, u8 width, u8 height, u8 numFrames);

	void updateSprite(u8 frameNumber, const u8* originalSprite);

public:

	u8 m_width;
	u8 m_height;
	u8 m_numFrames;
	std::vector<SpriteFrame> m_frames;
	const u8* m_originalSprite;
};

class SDLVideoFilterNewRenderer : public SDLVideoFilterBase
{
public:
	SDLVideoFilterNewRenderer(SDL_Renderer* renderer, 
						 const Resources* resources);

	bool init() override;
	void shutdown() override;
	void update(const GameData* gameData) override;
	void roomChanged(const GameData* gameData, u8 roomNumber, s8 transitionType);

private:

	void updateRegenSprite(u8 currentPlayerSpriteNumber);

	void drawDrops(const GameData* gameData, u32* framebuffer);

	void drawChamber(const GameData* gameData, u32* framebuffer);
	void drawTitleScreen(const GameData* gameData, u32* framebuffer);
	void drawTransition(const GameData* gameData, u32* framebuffer);
	void drawWipeTransition(const GameData* gameData, u32* framebuffer);
	void drawGetReadyScreen(const GameData* gameData, u32* framebuffer);

private:
	unsigned int m_framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
	unsigned int m_wipeFramebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

	// we build 32bit sprites from the raw 1bpp sprites
	Sprite m_dropSprite;
	Sprite m_ballSprite;
	Sprite m_playerSprite;
	Sprite m_playerSplatSprite;
	Sprite m_birdSprite;
	Sprite m_keySprite;
	Sprite m_diamondSprite;
	Sprite m_moneyBagSprite;
	Sprite m_regenSprite;
	Sprite m_characterFont;

	u8 m_regenSpriteBuffer[(PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS];

	std::vector<const Sprite*> m_pickUpSprites;

	const Resources* m_resources;

	typedef void (SDLVideoFilterNewRenderer::*DrawRoomFunction)(const GameData* gameData, u32* framebuffer);
	std::vector<DrawRoomFunction> m_drawRoomFunctions;
};

#endif