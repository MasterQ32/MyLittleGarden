#ifndef ENGINE_H
#define ENGINE_H

#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

extern SDL_Renderer * renderer;
extern SDL_Window * window;

using Sound = Mix_Chunk * ;
using Music = Mix_Music * ;
using Image = SDL_Texture * ;

void quit();

[[noreturn]] void die(char const * msg);

glm::vec2 map_to_screen(glm::vec2 pos);
glm::vec2 map_to_game(glm::vec2 pos);

Image LoadImage(char const * fileName);
void BlitImage(Image, glm::ivec2 pos);
Image CreateRenderTarget(int w, int h);

Sound LoadSound(char const * fileName);
void PlaySound(Sound sound);

Music LoadMusic(char const * fileName);
void PlayMusic(Music music);

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

template<typename T>
static inline T rng(T min, T max)
{
	return min + ((max - min) * rand()) / RAND_MAX;
}

#endif // ENGINE_H
