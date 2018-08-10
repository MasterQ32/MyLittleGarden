#ifndef ENGINE_H
#define ENGINE_H

#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

extern SDL_Renderer * renderer;
extern SDL_Window * window;

void quit();

[[noreturn]] void die(char const * msg);

glm::vec2 map_to_screen(glm::vec2 pos);
glm::vec2 map_to_game(glm::vec2 pos);

SDL_Texture * LoadImage(char const * fileName);
void BlitImage(SDL_Texture * texture, glm::ivec2 pos);
SDL_Texture * CreateRenderTarget(int w, int h);

class RenderTargetGuard
{
private:
	SDL_Texture * previous;
public:
	RenderTargetGuard(SDL_Texture * target) : previous(SDL_GetRenderTarget(renderer))
	{
		SDL_SetRenderTarget(renderer, target);
	}
	RenderTargetGuard(RenderTargetGuard const &) = delete;
	RenderTargetGuard(RenderTargetGuard &&) = delete;
	~RenderTargetGuard()
	{
		SDL_SetRenderTarget(renderer, this->previous);
	}
};

#endif // ENGINE_H
