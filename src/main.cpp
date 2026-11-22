#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logW, logH;
};

void cleanup(SDLState& state);

int main(int argc, char* argv[])
{
	SDLState state;

	state.width = 800;
	state.height = 800;
	state.logW = 640;
	state.logH = 320;

	//create the window
	state.window = SDL_CreateWindow("SDL is working", state.width, state.height, 0);
	if (!state.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", state.window);
		cleanup(state);
		return 1;
	}

	//create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
	}

	//load game assets
	SDL_Texture* idleTex = IMG_LoadTexture(state.renderer, RESOURCES_PATH "idle.png");

	//start the gameloop
	bool running = true;
	while (running)
	{
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

		//perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 30, 10, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(state.renderer);

		SDL_RenderTexture(state.renderer, idleTex, nullptr, nullptr);

		//swap the buffers
		SDL_RenderPresent(state.renderer);
	}

	SDL_DestroyTexture(idleTex);
	cleanup(state);
	return 0;
}

void cleanup(SDLState& state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}