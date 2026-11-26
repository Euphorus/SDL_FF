#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cmath>
#include <vector>

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height;
};

int main(int argc, char* argv[])
{
	SDLState state;

	state.width = 960;
	state.height = 540;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDl_INIT failed: %s", SDL_GetError());
		return 1;
	}

	state.window = SDL_CreateWindow("FF - FingerFlicker not FinalFantasy", state.width, state.height, SDL_WINDOW_RESIZABLE);
	state.renderer = SDL_CreateRenderer(state.window, NULL);
	SDL_SetRenderVSync(state.renderer, 1);

	SDL_FPoint center = { state.width / 2.0f, state.height / 2.0f };
	

	//setup game data
	
	uint64_t prevTime = ((double)SDL_GetTicks());

	bool running = true;
	while (running)
	{
		//get the latest time for the game
		uint64_t nowTime = ((double)SDL_GetTicks());
		float deltaTime = (nowTime - prevTime) / 1000.f; //to convert it to seconds

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

		//UPDATE goes here
		
		SDL_SetRenderDrawColor(state.renderer,235, 230, 220, 255);
		

		//DRAW goes here
		
		SDL_RenderPresent(state.renderer);
		

		SDL_RenderClear(state.renderer);
	}

	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
	return 0;
}

