#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>

#include "gameObject.h"

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logW, logH;
	const bool* keys;

	SDLState() : keys(SDL_GetKeyboardState(nullptr))
	{
	}
};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState
{
	std::array<std::vector<GameObject>, 2> layers;
	int playerIndex;

	GameState()
	{
		playerIndex = 0; //Will change this when we load map
	}
};


struct Resources  //it will help load and unload all of the game assets, so textures, sound files, pointers, animations
{
	const int ANIM_PLAYER_IDLE = 0; //index of the idle animation
	const int ANIM_PLAYER_RUN = 1; //index for the run animation
	std::vector<Animation> playerAnimation;

	std::vector<SDL_Texture*> textures;
	SDL_Texture *idleTex, *runTex, *brickTex, *grassTex, *groundTex, *panelTex;

	SDL_Texture* loadTexture(SDL_Renderer *renderer, const std::string& filepath)
	{
		//load game assets
		SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	void load(SDLState& state) //This function will handle loading or unloading all game assets. WTF!!!
	{
		playerAnimation.resize(5);
		playerAnimation[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		idleTex = loadTexture(state.renderer, RESOURCES_PATH "idle.png");

		playerAnimation[ANIM_PLAYER_RUN] = Animation(4, 0.6f);
		runTex = loadTexture(state.renderer, RESOURCES_PATH "run.png");

		//loading the ground-related textures
		brickTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/brick.png");
		grassTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/grass.png");
		groundTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/ground.png");
		panelTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/panel.png");
	}

	void unload()
	{
		for (SDL_Texture* tex : textures)
		{
			SDL_DestroyTexture(tex);
		}
	}
};


bool initialize(SDLState& state);
void cleanup(SDLState& state);
void drawOjbect(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime);
void update(SDLState& state, GameObject& obj, GameState& gs, Resources& res, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltatime);

int main(int argc, char* argv[])
{
	SDLState state;

	state.width = 1280;
	state.height = 720;
	state.logW = 640;
	state.logH = 320;

	if (!initialize(state))
	{
		return 1;
	}

	//load game assets
	Resources res;
	res.load(state);

	//setup game data
	GameState gs;
	createTiles(state, gs, res);
	uint64_t prevTime = ((double)SDL_GetTicks());
	
	//start the gameloop
	bool running = true;
	while (running)
	{
		uint64_t nowTime = ((double)SDL_GetTicks()); //convert it from milliseconds to seconds.
		float deltaTime = (nowTime - prevTime) / 1000.f;
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}

		//update all objects
		for (auto& layer : gs.layers)
		{
			for (GameObject& obj : layer)
			{
				update(state, obj, gs, res, deltaTime);
				//update the animation
				if (obj.currentAnimation != -1)
				{
					obj.animations[obj.currentAnimation].step(deltaTime);
				}
			}
		}
		

		//perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 30, 10, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(state.renderer);

		//draw all objects
		for (auto& layer : gs.layers)
		{
			for (GameObject& obj : layer)
			{
				drawOjbect(state, gs, obj, deltaTime);
			}
		}
		
		//swap the buffers
		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
	}

	res.unload();
	cleanup(state);
	return 0;
}

bool initialize(SDLState& state)
{
	bool initSuccess = true;

	//create the window
	state.window = SDL_CreateWindow("SDL is working", state.width, state.height, 0);
	if (!state.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", state.window);
		cleanup(state);
		initSuccess = false;
	}

	//create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		initSuccess = false;
	}

	//configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	return initSuccess;
}

void cleanup(SDLState& state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}

void drawOjbect(const SDLState& state, GameState& gs, GameObject& obj, float deltaTime)
{
	const float spriteSize = 32;
	float srcX = obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame() * spriteSize : 0.0f;

	SDL_FRect src{
		src.x = srcX,
		src.y = 0,
		src.w = spriteSize,
		src.h = spriteSize
	};

	SDL_FRect dst{
		dst.x = obj.position.x,
		dst.y = obj.position.y,
		dst.w = spriteSize,
		dst.h = spriteSize
	};

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	//SDL_RenderTexture(state.renderer, idleTex, &src, &dst);
	SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);

}

void update(SDLState& state, GameObject& obj, GameState& gs, Resources& res, float deltaTime)
{
	if (obj.dynamic)
	{
		//apply some gravity
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	}
	

	if (obj.type == ObjectType::player)
	{
		float currentDirection = 0;
		if (state.keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
		}
		if (state.keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
		}
		if (currentDirection)
		{
			obj.direction = currentDirection;
		}

		switch (obj.data.player.state)
		{
		case PlayerState::idle:
			if (currentDirection)
			{
				obj.data.player.state = PlayerState::running;
				obj.texture = res.runTex;
				obj.currentAnimation = res.ANIM_PLAYER_RUN;
			}
			else
			{
				//decelerate
				if (obj.velocity.x)
				{
					const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
					float amount = factor * obj.acceleration.x * deltaTime;
					if (std::abs(obj.velocity.x) < std::abs(amount))
					{
						obj.velocity.x = 0;
					}
					else
					{
						obj.velocity.x += amount;
					}
				}
			}
			break;

		case PlayerState::running:
			if (!currentDirection)
			{
				obj.data.player.state = PlayerState::idle;
				obj.texture = res.idleTex;
				obj.currentAnimation = res.ANIM_PLAYER_IDLE;
			}
			break;

		case PlayerState::jumping:
			break;
		}

		//add acceleration to velocity
		obj.velocity += currentDirection * obj.acceleration * deltaTime;
		if (std::abs(obj.velocity.x) > obj.maxSpeedX)
		{
			obj.velocity.x = currentDirection * obj.maxSpeedX;
		}
	}

	//add velocity to position
	obj.position += obj.velocity * deltaTime;

	//handle collision detection
	for (auto& layer : gs.layers)
	{

	}
}

void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& rectC, GameObject &objA, GameObject &objB, float deltaTime)
{
	//object we are checking
	if (objA.type == ObjectType::player)
	{
		//object it is colliding with
		switch (objB.type)
		{
			case ObjectType::level:
			{
				if (rectC.w < rectC.h)
				{
					//horizontal collision
					if (objA.velocity.x > 0) // going right
					{
						objA.position.x -= rectC.w;
					}
					else if (objA.velocity.x < 0) //going left
					{
						objA.position.x += rectC.w;
					}
					objA.velocity.x = 0;
				}
				else
				{
					//vertical collision
					if (objA.velocity.y > 0) // going down
					{
						objA.position.y -= rectC.h;
					}
					else if (objA.velocity.y < 0) // going up
					{
						objA.position.y += rectC.h;
					}
					objA.velocity.y = 0;
				}
				break;
			}
		}
	}
}

//my favourite part of pure programming in programming games
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltatime)
{
	SDL_FRect rectA{
		.x = a.position.x, .y = a.position.y,
		.w = TILE_SIZE, .h = TILE_SIZE
	};

	SDL_FRect rectB{
		.x = b.position.x, .y = b.position.y,
		.w = TILE_SIZE, .h = TILE_SIZE
	};
	SDL_FRect rectC{ 0 };

	if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC))
	{
		//found intersection, respond accordingly
		collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltatime);
	}
}

void createTiles(const SDLState& state, GameState& gs, const Resources& res)
{
	/*
	* 0 - Empty Tiles
	* 1 - Ground
	* 2 - Panel
	* 3 - Enemy
	* 4 - Player
	* 5 - Grass
	* 6 - Brick
	*/

	short map[MAP_ROWS][MAP_COLS] = {
		4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 0, 2, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	const auto createObject = [&state](int r, int c, SDL_Texture* tex, ObjectType type)
		{
			GameObject o;
			o.type = type;
			o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
			o.texture = tex;
			return o;
		};
	for (int r = 0; r < MAP_ROWS; r++)
	{
		for (int c = 0; c < MAP_COLS; c++)
		{
			switch (map[r][c])
			{
				case 1:
				{
					//Create the "ground" part of the level
					GameObject ground = createObject(r, c, res.groundTex, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(ground);
					break;
				}

				case 2:
				{
					//Create the "panel" part of the level
					GameObject panel = createObject(r, c, res.panelTex, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(panel);
					break;
				}


				case 4: //players
				{
					//Create our player
					GameObject player = createObject(r, c, res.idleTex, ObjectType::player);
					player.data.player = PlayerData();
					player.texture = res.idleTex;
					player.animations = res.playerAnimation;
					player.currentAnimation = res.ANIM_PLAYER_IDLE;
					player.acceleration = glm::vec2(300, 0);
					player.maxSpeedX = 100;
					player.dynamic = true;
					gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
					break;
				}

				case 5:
				{
					//Create the "grass" part of the level
					GameObject grass = createObject(r, c, res.grassTex, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(grass);
					break;
				}

				case 6:
				{
					//Create the "brick" part of the level
					GameObject brick = createObject(r, c, res.brickTex, ObjectType::level);
					gs.layers[LAYER_IDX_LEVEL].push_back(brick);
					break;
				}
			}
		}
	}
}