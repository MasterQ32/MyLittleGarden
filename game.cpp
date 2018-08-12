#include "game.hpp"
#include "palette.h"

#include <vector>
#include <array>
#include <algorithm>
#include <functional>

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

struct Color { Uint8 r, g, b; };

struct Particle
{
	int lifespan;
	vec2 pos;
	vec2 vel;
	vec2 accel;
	Color color;

	Particle() : lifespan(1), pos(), vel(), accel(), color{BLUE}
	{
	}
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
	Image mouse_cursors[7];

	Image ui_overlay;
	Image ui_catalog;

	Image planthole;
	Image font;
} textures;

struct
{
	Sound spray, click, dig, splash, exhume, plant;
} sounds;

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

static std::vector<Particle> particles;

static int32_t player_money = 5;

static std::array<int, plantTypes.size()> buyprice =
{
	4, 6, 9
};
static std::array<int, plantTypes.size()> sellprice =
{
	5, 8, 12
};

static void emit(int count, std::function<void(Particle&)> init)
{
	for(int i = 0; i < count; i++)
	{
		Particle p;
		init(p);
		particles.push_back(p);
	}
}

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
	fread(&player_money, sizeof player_money, 1, f);
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
	fwrite(&player_money, sizeof player_money, 1, f);
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
	textures.font  = LoadImage("data/font.png");

	sounds.click = LoadSound("data/click.wav");
	sounds.dig = LoadSound("data/dig.wav");
	sounds.splash = LoadSound("data/splash.wav");
	sounds.spray = LoadSound("data/spray.wav");
	sounds.exhume = LoadSound("data/exhume.wav");
	sounds.plant = LoadSound("data/plant.wav");

	fprintf(stderr, "IMPLEMENT \"Can't buy\"-sound!\n");

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

	for(auto & part : particles)
	{
		part.lifespan -= 1;
		part.pos += part.vel;
		part.vel += part.accel;
	}
	particles.erase(
		std::remove_if(
			particles.begin(),
			particles.end(),
			[](Particle const & p) { return p.lifespan <= 0; }),
		particles.end());
}

void tool_click(int id)
{
	if(mouse_pos.x == 0 || mouse_pos.x == 9)
		return;
	if((mouse_pos.y-1)%9==8)
		return;
	tool = Tool(id);
	gamestate = GardenView;

	PlaySound(sounds.click);
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

	PlaySound(sounds.dig);

	emit(5, [&](Particle & p)
	{
		p.pos = pos;
		p.vel = vec2(rng(-0.25, 0.25), rng(-0.5, -0.3));
		p.accel = vec2(0, 0.03);
		p.color = Color { BROWN };
		p.lifespan = 30 + rng(0, 30);
	});

	plants.push_back(Plant { -1, pos, 0.0, 0.0 });
}

void watering_can_click(ivec2 pos)
{
	emit(10, [&](Particle & p)
	{
		p.pos = pos;
		p.vel = vec2(rng(-0.2, -0.01), rng(-0.1, 0.3));
		p.color = Color { BLUE };
		p.lifespan = 20 + rng(0, 30);
	});

	PlaySound(sounds.splash);

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
	auto const & type = clicked->type();
	if(clicked->growth < type.stages.back().growth)
		return;
	player_money += sellprice[clicked->_type];
	plants.erase(clicked);
	PlaySound(sounds.exhume);
}

void fertilizer_click(ivec2 pos)
{
	emit(7, [&](Particle & p)
	{
		p.pos = pos;
		p.vel = 0.3f * normalize(vec2(-1.0, rng(-0.5, 1.0)));
		p.color = Color { INDIGO };
		p.lifespan = 20;
	});
	PlaySound(sounds.spray);
}

void seeds_click(ivec2 pos)
{
	auto clicked = get_clicked(pos);
	if(clicked == plants.end())
		return;
	if(clicked->_type != -1)
		return;
	player_money -= buyprice[seedtype];
	clicked->_type = int(seedtype);
	tool = Hand;
	PlaySound(sounds.plant);
}

void catalog_click()
{
	PlaySound(sounds.click);
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
			if(key == SDLK_c)
				catalog_click();

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
						if(buyprice[i] <= player_money)
						{
							seedtype = i;
							tool = Seeds;
							gamestate = GardenView;
							PlaySound(sounds.click);
						}
						else
						{
							fprintf(stderr, "Play can't buy-sound here!\n");
						}
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

	for(auto const & p : particles)
	{
		SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, 0xFF);
		SDL_RenderDrawPoint(renderer, int(p.pos.x + 0.5f), int(p.pos.y + 0.5f));
	}
}

// pos.x is right aligned
static void render_num(ivec2 pos, int number)
{
	pos.x -= 4;
	if(number == 0)
	{
		BlitImagePortion(textures.font,pos,SDL_Rect { 0, 0, 4, 5 });
	}
	else
	{
		while(number > 0)
		{
			BlitImagePortion(textures.font,pos,SDL_Rect { 4*(number%10), 0, 4, 5 });
			number /= 10;
			pos.x -= 4;
		}
	}
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
	{
		BlitImage(textures.ui_catalog, ivec2());

		render_num(ivec2(74, 1), player_money);
		render_num(ivec2(74, 11), buyprice[0]);
		render_num(ivec2(74, 21), buyprice[1]);
		render_num(ivec2(74, 31), buyprice[2]);
	}

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
