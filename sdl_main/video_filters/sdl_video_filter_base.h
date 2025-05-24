#ifndef SDL_VIDEO_FILTER_BASE
#define SDL_VIDEO_FILTER_BASE

#include <SDL3/SDL.h>
#include "..\..\game\resource_types.h"
#include "..\..\game\game.h"

class SDLVideoFilterBase
{
public:
	SDLVideoFilterBase(SDL_Renderer *renderer, const Resources* resources) 
		: m_renderer(renderer), 
		  m_outputTexture(nullptr)
	{

	}

	virtual ~SDLVideoFilterBase() 
	{ 
		shutdown(); 
	}

	virtual bool init() = 0;

	virtual void shutdown()
	{
		if (m_outputTexture != nullptr)
		{
			SDL_DestroyTexture(m_outputTexture);
			m_outputTexture = nullptr;
		}
	}

	virtual void roomChanged(u8 roomNumber, s8 transitionType) {};
	virtual void update(const GameData* gameData) = 0;

	SDL_Texture* getOutputTexture() { return m_outputTexture; }

protected:
	SDL_Renderer* m_renderer;
	SDL_Texture* m_outputTexture;
};

#endif