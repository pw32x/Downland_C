#ifndef SDL_GAME_RENDERER
#define SDL_GAME_RENDERER

#include <SDL3/SDL.h>
#include "..\..\game\resource_types.h"
#include "..\..\game\game.h"
#include <vector>

// This is an example of using the game state to render
// a game frame, instead of using the raw simulated
// CoCo framebuffer. With this, we can start looking into 
// plugging in different kinds of renderers.

extern "C"
{
#include "..\..\game\base_defines.h"
}

typedef std::vector<dl_u32> SpriteFrame;

class Sprite
{
public:
	Sprite(const dl_u8* orginalSprite, dl_u8 width, dl_u8 height, dl_u8 numFrames);

	void updateSprite(dl_u8 frameNumber, const dl_u8* originalSprite);

public:

	dl_u8 m_width;
	dl_u8 m_height;
	dl_u8 m_numFrames;
	std::vector<SpriteFrame> m_frames;
	const dl_u8* m_originalSprite;
};

class GameRenderer
{
public:
	GameRenderer(SDL_Renderer* renderer, 
						     const Resources* resources);

	~GameRenderer();

	void shutdown();
	void update(const GameData* gameData);
	void roomChanged(const GameData* gameData, dl_u8 roomNumber, dl_s8 transitionType);

	SDL_Texture* getOutputTexture() { return m_outputTexture; }

private:

	bool init();

	void updateRegenSprite(dl_u8 currentPlayerSpriteNumber);

	void drawDrops(const GameData* gameData, dl_u32* framebuffer);

	void drawChamber(const GameData* gameData, dl_u32* framebuffer);
	void drawTitleScreen(const GameData* gameData, dl_u32* framebuffer);
	void drawTransition(const GameData* gameData, dl_u32* framebuffer);
	void drawWipeTransition(const GameData* gameData, dl_u32* framebuffer);
	void drawGetReadyScreen(const GameData* gameData, dl_u32* framebuffer);

private:
	SDL_Renderer* m_renderer;
	SDL_Texture* m_outputTexture;

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

	dl_u8 m_regenSpriteBuffer[(PLAYER_SPRITE_WIDTH / 8) * PLAYER_SPRITE_ROWS];

	std::vector<const Sprite*> m_pickUpSprites;

	const Resources* m_resources;

	typedef void (GameRenderer::*DrawRoomFunction)(const GameData* gameData, dl_u32* framebuffer);
	std::vector<DrawRoomFunction> m_drawRoomFunctions;


};

#endif