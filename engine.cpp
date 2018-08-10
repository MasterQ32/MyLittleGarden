#include "engine.h"
#include "game.hpp"

SDL_Renderer * renderer;
SDL_Window * window;

void die(char const * msg)
{
	fprintf(stderr, "DED: %s\n", msg);
	fflush(stderr);
	exit(EXIT_FAILURE);
}

static bool wants_quit = false;

static glm::ivec2 screen_size;

int main()
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		die(SDL_GetError());
	atexit(SDL_Quit);

	if(IMG_Init(IMG_INIT_PNG) == 0)
		die(IMG_GetError());
	atexit(IMG_Quit);

	window = SDL_CreateWindow(
		"My Little Garden - Growing Plants Is Magic!",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_SHOWN);
	if(window == nullptr)
		die(SDL_GetError());

	renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if(renderer == nullptr)
		die(SDL_GetError());

	SDL_Texture * renderTarget = CreateRenderTarget(80, 60);

	SDL_ShowCursor(0);

	game_init();

	auto next_update = SDL_GetTicks();
	auto const frametime = 1000.0 / 60.0;
	do
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
				quit();

			if(e.type == SDL_WINDOWEVENT)
			{
				SDL_GetWindowSize(window, &screen_size.x, &screen_size.y);
			}

			game_do_event(e);
		}

		auto now = SDL_GetTicks();
		while(next_update < now)
		{
			game_update();
			next_update += frametime;
		}

		{
			RenderTargetGuard _g(renderTarget);
			game_render();
		}

		SDL_RenderCopy(
			renderer,
			renderTarget,
			nullptr,
			nullptr);

		SDL_RenderPresent(renderer);

	} while(!wants_quit);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}


void quit()
{
	wants_quit = true;
}

glm::vec2 map_to_screen(glm::vec2 pos)
{
	pos *= glm::vec2(screen_size) / glm::vec2(80,60);
	return pos;
}

glm::vec2 map_to_game(glm::vec2 pos)
{
	pos *= glm::vec2(80,60) / glm::vec2(screen_size);
	return pos;
}


SDL_Texture * LoadImage(char const * fileName)
{
	auto * tex = IMG_LoadTexture(renderer, fileName);
	if(!tex)
		die(IMG_GetError());
	return tex;
}


void BlitImage(SDL_Texture * texture, glm::ivec2 pos)
{
	SDL_Rect rect { pos.x, pos.y, 8, 8 };

	SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);

	SDL_RenderCopy(
		renderer,
		texture,
		nullptr,
		&rect);
}

SDL_Texture * CreateRenderTarget(int w, int h)
{
	auto * tex = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET,
			w, h);
	if(tex == nullptr)
		die(SDL_GetError());
	return tex;
}
