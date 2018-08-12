#include "game.hpp"
#include "palette.h"

#include <vector>
#include <array>
#include <algorithm>

using namespace glm;

enum Tool
{
	Hand,
	Shovel,
	WateringCan,
	Pot,
	Fertilizer,
	Seeds
};

enum GameState
{
	GardenView,
	CatalogView
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

static std::array<PlantType,3> plantTypes;

static Tool tool;
static uint seedtype;

static GameState gamestate;

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
	SDL_Texture * mouse_cursors[7];

	SDL_Texture * ui_overlay;
	SDL_Texture * ui_catalog;

	SDL_Texture * planthole;
} textures;

static ivec2 tool_offsets[7] =
{
	ivec2(0,0),
    ivec2(0,0),
    ivec2(0,2),
    ivec2(2,1),
    ivec2(2,2),
    ivec2(3,3),
};

static std::vector<Plant> plants;

static SDL_Texture * acreTarget;

static glm::ivec2 mouse_pos;
static glm::ivec2 scroll_offset;

static bool is_scrolling;

bool game_has_save()
{
	FILE * f = fopen("savegame.dat", "rb");
	if(f != nullptr)
		fclose(f);
	return (f != nullptr);
}

void game_load()
{
	FILE * f = fopen("savegame.dat", "rb");
	if(f == nullptr)
		die("Could not open savegame.dat");

	uint32_t count, magic;
	fread(&magic, sizeof magic, 1, f);
	if(magic != 0xBADEAFFE)
		die("Failed to load game: Is not a badeaffe!");

	fread(&count, sizeof count, 1, f);
	for(uint32_t i = 0; i < count; i++)
	{
		Plant plant;
		fread(&plant, sizeof(plant), 1, f);
		plants.push_back(plant);
	}
	fclose(f);
}

void game_save()
{
	FILE * f = fopen("savegame.dat", "wb");
	if(f == nullptr)
		die("Could not open savegame.dat");

	uint32_t count = uint32_t(plants.size());
	uint32_t magic = 0xBADEAFFE;
	fwrite(&magic, sizeof magic, 1, f);
	fwrite(&count, sizeof count, 1, f);

	fwrite(plants.data(), sizeof(Plant), plants.size(), f);

	fclose(f);
}

void game_init()
{
	textures.mouse_cursors[Hand] = LoadImage("data/mouse_hand.png");
	textures.mouse_cursors[Shovel] = LoadImage("data/mouse_shovel.png");
	textures.mouse_cursors[WateringCan] = LoadImage("data/mouse_watering_can.png");
	textures.mouse_cursors[Pot] = LoadImage("data/mouse_pot.png");
	textures.mouse_cursors[Fertilizer] = LoadImage("data/mouse_fertilizer.png");
	textures.mouse_cursors[Seeds] = LoadImage("data/seeds.png");
	textures.ui_overlay = LoadImage("data/ui_overlay.png");
	textures.ui_catalog = LoadImage("data/catalog.png");
	textures.planthole  = LoadImage("data/planthole.png");

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

	plantTypes[1] = PlantType
	{
		"Rotbeer-Strauch",
		0.01,
		{
	        GrowStage {
				0.0, ivec2(3, 11),
				LoadImage("data/plant1_stage0.png"),
			},
	        GrowStage {
				1.0, ivec2(3, 11),
				LoadImage("data/plant1_stage1.png"),
			},
	        GrowStage {
				2.0, ivec2(3, 11),
				LoadImage("data/plant1_stage2.png"),
			},
	        GrowStage {
				3.0, ivec2(3, 11),
				LoadImage("data/plant1_stage3.png"),
			},
	        GrowStage {
				4.0, ivec2(3, 11),
				LoadImage("data/plant1_stage4.png"),
			},
	        GrowStage {
				5.0, ivec2(3, 11),
				LoadImage("data/plant1_stage5.png"),
			},
	        GrowStage {
				6.0, ivec2(3, 11),
				LoadImage("data/plant1_stage6.png"),
			},
	        GrowStage {
				7.0, ivec2(3, 11),
				LoadImage("data/plant1_stage7.png"),
			},
	        GrowStage {
				8.0, ivec2(3, 11),
				LoadImage("data/plant1_stage8.png"),
			},
		}
	};

	plantTypes[2] = PlantType
	{
		"Citromben-Baum",
		0.01,
		{
	        GrowStage {
				0.0, ivec2(3, 11),
				LoadImage("data/plant2_stage0.png"),
			},
	        GrowStage {
				1.0, ivec2(3, 11),
				LoadImage("data/plant2_stage1.png"),
			},
	        GrowStage {
				2.0, ivec2(3, 11),
				LoadImage("data/plant2_stage2.png"),
			},
	        GrowStage {
				3.0, ivec2(3, 11),
				LoadImage("data/plant2_stage3.png"),
			},
	        GrowStage {
				4.0, ivec2(3, 11),
				LoadImage("data/plant2_stage4.png"),
			},
	        GrowStage {
				5.0, ivec2(3, 11),
				LoadImage("data/plant2_stage5.png"),
			},
	        GrowStage {
				6.0, ivec2(3, 11),
				LoadImage("data/plant2_stage6.png"),
			},
	        GrowStage {
				7.0, ivec2(3, 11),
				LoadImage("data/plant2_stage7.png"),
			},
	        GrowStage {
				8.0, ivec2(3, 11),
				LoadImage("data/plant2_stage8.png"),
			},
		}
	};

#if defined(RELEASE)
	PlayMusic(LoadMusic("data/truth_in_the_stones.mp3"));
#endif

	if(game_has_save())
		game_load();
	else
		plants.clear();
	/*
	plants.push_back(Plant { -1, ivec2(10, 20), 0.0, 0.0 });
	plants.push_back(Plant {
		0, ivec2(20, 20), 0.0, 3.0
	});
	plants.push_back(Plant {
		1, ivec2(30, 20), 0.0, 4.0
	});
	plants.push_back(Plant {
		2, ivec2(40, 20), 0.0, 2.0
	});
	*/
}

void game_shutdown()
{
	game_save();
}

void game_update()
{
	std::sort(
		plants.begin(), plants.end(),
		[](Plant const & l, Plant const & r)
		{
			return l.position.y < r.position.y;
		});
	// Update all plants
	for(auto & plant : plants)
	{
		if(plant._type < 0)
			continue;
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
	if((mouse_pos.y-1)%9==8)
		return;
	tool = Tool(id);
	gamestate = GardenView;
}

// 4 pixels distance
static float mouse_sensitivity = 4.0;

static auto get_clicked(ivec2 pos)
{
	auto nearest = plants.end();
	float dist = std::numeric_limits<decltype(dist)>::max();
	for(auto it = plants.begin(); it != plants.end(); it++)
	{
		auto d = distance(vec2(pos), vec2(it->position));
		if(d > dist)
			continue;
		if(d > mouse_sensitivity)
			continue;
		nearest = it;
		dist = d;
	}
	return nearest;
}

void hand_click(ivec2)
{
	is_scrolling = true;
}

void shovel_click(ivec2 pos)
{
	auto clicked = get_clicked(pos);
	if(clicked != plants.end())
		return;
	plants.push_back(Plant { -1, pos, 0.0, 0.0 });
}

void watering_can_click(ivec2 pos)
{
	auto clicked = get_clicked(pos);
	if(clicked == plants.end())
		return;
	if(clicked->_type == -1)
		return;
	clicked->watering += rng(1.78, 2.44);
}

void pot_click(ivec2 pos)
{
	auto clicked = get_clicked(pos);
	if(clicked == plants.end())
		return;
	if(clicked->_type == -1)
		return;
	plants.erase(clicked);
}

void fertilizer_click(ivec2)
{

}

void seeds_click(ivec2 pos)
{
	auto clicked = get_clicked(pos);
	if(clicked == plants.end())
		return;
	if(clicked->_type != -1)
		return;
	clicked->_type = int(seedtype);
	tool = Hand;
}

void catalog_click()
{
	if(gamestate == CatalogView)
		gamestate = GardenView;
	else
		gamestate = CatalogView;
}

void game_do_event(SDL_Event const & ev)
{
	switch(ev.type)
	{
		case SDL_KEYDOWN:
		{
			auto key = ev.key.keysym.sym;
			if(key == SDLK_ESCAPE)
				tool = Hand;

			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if(ev.button.button == SDL_BUTTON_RIGHT)
			{
				tool = Hand;
				break;
			}

			mouse_pos = map_to_game(ivec2(ev.motion.x, ev.motion.y));
			if(mouse_pos.y > 0 && mouse_pos.x < 10 && mouse_pos.y < 46)
			{
				// TODO: Fix off-by-one
				tool_click((mouse_pos.y - 1) / 9);
			}
			else if(mouse_pos.x < 10 && mouse_pos.y >= 50)
			{
				if(mouse_pos.x%10 == 0 && mouse_pos.x%10 == 9)
					break;
				if(mouse_pos.y%10 == 0 && mouse_pos.y%10 == 9)
					break;
				catalog_click();
			}
			else if(mouse_pos.x >= 10 && mouse_pos.x < 79 && mouse_pos.y < 59)
			{
				if(gamestate == GardenView)
				{
					auto pos = mouse_pos - ivec2(10, 0) + scroll_offset;
					switch(tool)
					{
					case Hand: hand_click(pos); break;
					case Shovel: shovel_click(pos); break;
					case WateringCan: watering_can_click(pos); break;
					case Pot: pot_click(pos); break;
					case Fertilizer: fertilizer_click(pos); break;
					case Seeds: seeds_click(pos); break;
					}
				}
				else if(gamestate == CatalogView)
				{
					std::array<SDL_Rect,3> items =
					{
					    SDL_Rect { 11,  8, 9, 9 },
					    SDL_Rect { 11, 18, 9, 9 },
					    SDL_Rect { 11, 28, 9, 9 },
					};
					for(unsigned int i = 0; i < items.size(); i++)
					{
						if(!contains(items[i], mouse_pos))
							continue;
						seedtype = i;
						tool = Seeds;
						gamestate = GardenView;
						break;
					}
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
	if(plant._type < 0)
	{
		BlitImage(
			textures.planthole,
			plant.position - ivec2(2,1));
		return;
	}

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

	if(gamestate == GardenView)
		BlitImage(acreTarget, ivec2(10, 0) - scroll_offset);

	BlitImage(textures.ui_overlay, ivec2());

	SDL_SetRenderDrawColor(renderer, WHITE, 0xFF);
	SDL_RenderDrawLine(renderer, 10 + scroll_offset.x, 59, 13 + scroll_offset.x, 59);
	SDL_RenderDrawLine(renderer, 79, scroll_offset.y, 79, scroll_offset.y + 3);

	if(gamestate == CatalogView)
		BlitImage(textures.ui_catalog, ivec2());

	BlitImage(
		textures.mouse_cursors[int(tool)],
		mouse_pos - tool_offsets[int(tool)]);
}

void game_render()
{
	if(gamestate == GardenView)
		render_acre();
	render_ui();
}
