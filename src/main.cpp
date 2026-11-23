#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>

#include "animation.h"

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logW, logH;
};

bool initialize(SDLState& state);
void cleanup(SDLState& state);

struct Resources  //it will help load and unload all of the game assets, so textures, sound files, pointers, animations
{
	const int ANIM_PLAYER_IDLE = 0; //index of the idle animation
	std::vector<Animation> playerAnimation;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* idleTex;

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
	}

	void unload()
	{
		for (SDL_Texture* tex : textures)
		{
			SDL_DestroyTexture(tex);
		}
	}
};

int main(int argc, char* argv[])
{
	SDLState state;

	state.width = 800;
	state.height = 800;
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
	const bool* keys = SDL_GetKeyboardState(nullptr);
	float playerX = 0;
	const float floor = state.logH;
	uint64_t prevTime = ((double)SDL_GetTicks());
	bool flipHorizontal = false;

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

		//handle movement
		float moveAmount = 0;
		if (keys[SDL_SCANCODE_A])
		{
			moveAmount += -75.f;
			flipHorizontal = true;
		}
		if (keys[SDL_SCANCODE_D])
		{
			moveAmount += 75.f;
			flipHorizontal = false;
		}
		playerX += moveAmount * deltaTime;

		//perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 30, 10, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(state.renderer);

		const float spriteSize = 32;

		SDL_FRect src{
			src.x = 0,
			src.y = 0,
			src.w = spriteSize,
			src.h = spriteSize
		};

		SDL_FRect dst{
			dst.x = playerX,
			dst.y = floor - spriteSize,
			dst.w = spriteSize,
			dst.h = spriteSize
		};

		//SDL_RenderTexture(state.renderer, idleTex, &src, &dst);
		SDL_RenderTextureRotated(state.renderer, res.idleTex, &src, &dst, 0, nullptr, (flipHorizontal) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

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