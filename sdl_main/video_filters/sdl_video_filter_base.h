#ifndef SDL_VIDEO_FILTER_BASE
#define SDL_VIDEO_FILTER_BASE

#include <SDL3/SDL.h>

class SDLVideoFilterBase
{
public:
	SDLVideoFilterBase(SDL_Renderer *renderer) 
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

	virtual void update(unsigned char* gameFramebuffer) = 0;

	SDL_Texture* getOutputTexture() { return m_outputTexture; }

protected:
	SDL_Renderer* m_renderer;
	SDL_Texture* m_outputTexture;
};

#endif