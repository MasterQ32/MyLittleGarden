#include "game.hpp"
#include "palette.h"

#include <vector>
#include <array>

using namespace glm;

enum Tool
{
	Hand,
	Shovel,
	WateringCan,
	Pot,
	Fertilizer
};

struct GrowStage
{
	double growth;
	ivec2 origin;
	SDL_Texture * graphics;
};

struct PlantType
{
	char const * name;
	double growspeed;
	std::vector<GrowStage> stages;
};

static std::array<PlantType,1> plantTypes;

static Tool tool;

struct Plant
{
	int _type;
	ivec2 position;
	double growth;
	double watering;

	PlantType const & type() const
	{
		return plantTypes[_type];
	}
};

struct
{
	SDL_Texture * mouse_cursors[6];

	SDL_Texture * ui_overlay;
} textures;

static std::vector<Plant> plants;

static SDL_Texture * acreTarget;

static glm::ivec2 mouse_pos;
static glm::ivec2 scroll_offset;

static bool is_scrolling;

void game_init()
{
	textures.mouse_cursors[Hand] = LoadImage("data/mouse_hand.png");
	textures.mouse_cursors[Shovel] = LoadImage("data/mouse_shovel.png");
	textures.mouse_cursors[WateringCan] = LoadImage("data/mouse_watering_can.png");
	textures.mouse_cursors[Pot] = LoadImage("data/mouse_pot.png");
	textures.mouse_cursors[Fertilizer] = LoadImage("data/mouse_fertilizer.png");
	textures.ui_overlay = LoadImage("data/ui_overlay.png");

	acreTarget = CreateRenderTarget(69 + 65, 59 + 55);

	plantTypes[0] = PlantType
	{
		"Insel-Brulie",
		0.01,
		{
	        GrowStage {
				0.0, ivec2(3, 11),
				LoadImage("data/plant0_stage0.png"),
			},
	        GrowStage {
				1.0, ivec2(3, 11),
				LoadImage("data/plant0_stage1.png"),
			},
	        GrowStage {
				2.0, ivec2(3, 11),
				LoadImage("data/plant0_stage2.png"),
			},
	        GrowStage {
				3.0, ivec2(3, 11),
				LoadImage("data/plant0_stage3.png"),
			},
	        GrowStage {
				4.0, ivec2(3, 11),
				LoadImage("data/plant0_stage4.png"),
			},
		}
	};


	plants.push_back(Plant {
		0, ivec2(20, 20), 0.0, 10.0
	});
}

void game_update()
{
	// Update all plants
	for(auto & plant : plants)
	{
		auto delta = min(plant.watering, plant.type().growspeed);
		if(delta > 0)
		{
			plant.growth += delta;
			plant.watering -= delta;
		}
	}
}

void tool_click(int id)
{
	if(mouse_pos.x == 0 || mouse_pos.x == 9)
		return;
	if(mouse_pos.y%10==0 || mouse_pos.y%10==9)
		return;
	tool = Tool(id);
}

void hand_click()
{
	is_scrolling = true;
}

void game_do_event(SDL_Event const & ev)
{
	switch(ev.type)
	{
		case SDL_MOUSEBUTTONDOWN:
		{
			mouse_pos = map_to_game(ivec2(ev.motion.x, ev.motion.y));
			if(mouse_pos.x < 10 && mouse_pos.y < 46)
			{
				tool_click(mouse_pos.y / 10);
			}
			else if(mouse_pos.x >= 10 && mouse_pos.x < 79 && mouse_pos.y < 59)
			{
				switch(tool)
				{
				case Hand: hand_click(); break;
				case Shovel: break;
				case WateringCan: break;
				case Pot: break;
				case Fertilizer: break;
				}
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			auto prev = mouse_pos;
			mouse_pos = map_to_game(ivec2(ev.motion.x, ev.motion.y));
			auto delta = mouse_pos - prev;
			if(is_scrolling)
			{
				scroll_offset = clamp(
					scroll_offset - delta,
					ivec2(0,0),
					ivec2(65, 55));
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			is_scrolling = false;
			break;
		}
	}
}

static void draw_plant(Plant const & plant)
{
	GrowStage const * stage = &plant.type().stages[0];
	for(auto const & st : plant.type().stages)
	{
		if(plant.growth >= st.growth)
			stage = &st;
	}
	BlitImage(
		stage->graphics,
		plant.position - stage->origin);
}

static void render_acre()
{
	RenderTargetGuard _g(acreTarget);

	SDL_SetRenderDrawColor(renderer, DARK_GREEN, 0xFF);
	SDL_RenderClear(renderer);

	for(auto const & plant : plants)
		draw_plant(plant);
}

static void render_ui()
{
	SDL_SetRenderDrawColor(renderer, RED, 0xFF);
	SDL_RenderClear(renderer);

	BlitImage(acreTarget, ivec2(10, 0) - scroll_offset);

	BlitImage(textures.ui_overlay, ivec2());

	SDL_SetRenderDrawColor(renderer, WHITE, 0xFF);
	SDL_RenderDrawLine(renderer, 10 + scroll_offset.x, 59, 13 + scroll_offset.x, 59);
	SDL_RenderDrawLine(renderer, 79, scroll_offset.y, 79, scroll_offset.y + 3);

	BlitImage(textures.mouse_cursors[int(tool)], mouse_pos);
}

void game_render()
{
	render_acre();
	render_ui();
}
