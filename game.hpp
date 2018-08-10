#ifndef GAME_HPP
#define GAME_HPP

#include "engine.h"

void game_init();

void game_update();

void game_render();

void game_do_event(SDL_Event const & ev);

#endif // GAME_HPP
