#include "game.h"
#include "draw.h"
#include <algorithm>

bool Game::init()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("FF - FingerFlicker not FinalFantasy", cfg::ScreenW, cfg::ScreenH, 0);
	render = SDL_CreateRenderer(window, nullptr);
	if (!window || !render)
	{
		SDL_Log("Window/render creation failed: %s", SDL_GetError());
		return false;
	}
	SDL_SetRenderVSync(render, 1);

	return true;
}

void Game::run()
{
	running = true;
	uint64_t prevTime = ((double)SDL_GetTicks());

	while (running)
	{
		//get the latest time from the game
		uint64_t nowTime = ((double)SDL_GetTicks());
		float dt = std::min((nowTime - prevTime) / 1000.f, 0.05f);
		prevTime = nowTime;

		bool flickPressed = false;
		handleEvents(flickPressed);
		update(dt, flickPressed);
		rendertypeshi();
	}
}

void Game::shutdown()
{
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Game::reset()
{
	player = Player{};
	enemies.clear();
	effects = Effect{};
	health = cfg::StartHealth;
	score = 0;
	gameTime = 0.0f;
	state = GameState::Playing;
}

void Game::handleEvents(bool& flickPressed)
{
	SDL_Event event{ 0 };
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_KEY_DOWN:
				if (event.key.key == SDLK_ESCAPE)
					running = false;
				if (event.key.key == SDLK_SPACE && !event.key.repeat)
					flickPressed = true;
				break;
		}
	}
}

void Game::update(float dt, bool flickPressed)
{
	effects.update(dt);

	//A / D rotate the hand 
	const bool* keys = SDL_GetKeyboardState(nullptr);
	float dir = 0.0f;
	if (keys[SDL_SCANCODE_A]) dir -= 1.0f;
	if (keys[SDL_SCANCODE_D]) dir += 1.0f;
	player.update(dt, dir);

	if (state == GameState::GameOver)
	{
		if (flickPressed)
			reset();
		return;
	}

	gameTime += dt;

	//flick
	if (flickPressed && player.canFlick())
	{
		player.startFlick();
		int flicked = enemies.flick(player);
		if (flicked > 0)
		{
			score += flicked;
			effects.triggerShake();
		}
	}

	//enemies move, and tell us if they bit the player
	int bites = enemies.update(dt, player, gameTime);
	if (bites > 0)
	{
		health -= bites;
		effects.triggreFlash();
	}

	if (health <= 0)
		state = GameState::GameOver;
}

void Game::rendertypeshi()
{
	//background: flashes red after a bite
	float f = effects.flash();
	SDL_SetRenderDrawColor(render, (Uint8)(30 + 130 * f), 30, 40, 255);
	SDL_RenderClear(render);

	//the world is drawn through the (possibly shaken) viewport
	SDL_Rect view = effects.shakeViewport();
	SDL_SetRenderViewport(render, &view);
	player.draw(render);
	enemies.draw(render);
	SDL_SetRenderViewport(render, nullptr);  //HUD stays still

	//HUD
	char buf[64];
	SDL_SetRenderDrawColor(render, 240, 240, 240, 255);
	SDL_snprintf(buf, sizeof(buf), "Score: %d", score);
	drawText(render, 12, 12, 2.f, buf);

	for (int i = 0; i < health; ++i)
	{
		SDL_FRect heart = { cfg::ScreenW - 30.f - i * 28.f, 14.f, 20.f, 20.f };
		SDL_SetRenderDrawColor(render, 210, 50, 60, 255);
		SDL_RenderFillRect(render, &heart);
	}

	if (state == GameState::GameOver)
	{
		SDL_SetRenderDrawColor(render, 240, 240, 240, 255);
		drawTextCentered(render, 160, 4.0f, "GAME OVER");
		drawTextCentered(render, 230, 2.0f, "Press SPACE to restart");
	}

	SDL_RenderPresent(render);
}