#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cmath>
#include <vector>

#define SCR_WIDTH 640
#define SCR_HEIGHT 480
bool flickPress = false;

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
};

struct Enemy {
	SDL_FPoint pos;
	bool alive = true;
};

//Pick a random spot just outside one of the 4 screen edges
SDL_FPoint randomSpawnPoints()
{
	const float margin = 30.f;
	switch (SDL_rand(4))
	{
		case 0:
			return { SDL_randf() * SCR_WIDTH, -margin };
			break;

		case 1:
			return { SDL_randf() * SCR_WIDTH, SCR_HEIGHT + margin };
			break;

		case 2:
			return { -margin, SDL_randf() * SCR_HEIGHT };
			break;

		default:
			return { SCR_WIDTH + margin, SDL_randf() * SCR_HEIGHT };
			break;
	}
}

void drawCircle(SDL_Renderer* r, SDL_FPoint center, float radius);

int main(int argc, char* argv[])
{
	SDLState state;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDl_INIT failed: %s", SDL_GetError());
		return 1;
	}

	state.window = SDL_CreateWindow("FF - FingerFlicker not FinalFantasy", SCR_WIDTH, SCR_HEIGHT, 0);
	state.renderer = SDL_CreateRenderer(state.window, NULL);
	SDL_SetRenderVSync(state.renderer, 1);

	//player (the rotating square)
	SDL_FPoint center = { SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f };
	const float half = 40.f;
	const float playerRadius = half;
	const float turnSpeed = 80.f * SDL_PI_F / 180.f;
	float angle = 0;
	const float pointerLen = 90.f;
	const float flickRadius = 40.f;
	const float flickCooldown = 0.25f;
	float flickTimer = 0.0f;
	

	//enemy (the something something something)
	std::vector<Enemy> enemies;
	const float enemySize = 24.f;
	const float enemyRadius = enemySize / 2.0f;
	const float enemySpeed = 40.f;
	const float spawnInterval = 2.f;
	float spawnTimer = 0.0f;

	//setup game data
	
	uint64_t prevTime = ((double)SDL_GetTicks());

	bool running = true;
	while (running)
	{
		//get the latest time for the game
		uint64_t nowTime = ((double)SDL_GetTicks());
		float deltaTime = (nowTime - prevTime) / 1000.f; //to convert it to seconds
		prevTime = nowTime;
		deltaTime = std::min(deltaTime, 0.05f);

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
						flickPress = false;
					break;
			}

		}

		//create placeholder inputs for testing purposes
		const bool* keys = SDL_GetKeyboardState(nullptr);
		float dir = 0.0f;
		if (keys[SDL_SCANCODE_A]) dir -= 1.f;
		if (keys[SDL_SCANCODE_D]) dir += 1.f;
		

		//udpate the angle
		angle += dir * turnSpeed * deltaTime;
		float c = std::cos(angle), s = std::sin(angle);
		SDL_FPoint tip = { center.x + pointerLen * c, center.y + pointerLen * s };
		if (flickTimer > 0.f) flickTimer -= deltaTime;

		//spawn enemies on a timer
		spawnTimer -= deltaTime;
		if (spawnTimer <= 0.0f)
		{
			Enemy en;
			en.pos = randomSpawnPoints();
			enemies.push_back(en);
			spawnTimer = spawnInterval;
		}

		//move the enemies towards the player and check for collision
		for (auto& en : enemies)
		{
			//direction = (target - position), then shrink it to length 1
			float dx = center.x - en.pos.x;
			float dy = center.y - en.pos.y;
			float dist = std::sqrt(dx * dx + dy * dy);

			if (dist > 0.0001f)
			{
				en.pos.x += (dx / dist) * enemySpeed * deltaTime;
				en.pos.y += (dy / dist) * enemySpeed * deltaTime;
			}

			//collision: circle vs circle. they touch when
			//the distance between centers is smaller than the two
			//radii added together
			if (dist <= playerRadius + enemyRadius)
			{
				SDL_Log("HIT! an enemy touched the player.");
				en.alive = false;
			}
		}

		//remove enemies that hit (done after the loop we don't erase while loading
		enemies.erase(
			std::remove_if(enemies.begin(), enemies.end(),
				[](const Enemy& en) {return !en.alive; }),
			enemies.end()
		);

		//debug log for checking the angle
		if (dir != 0.0f)
		{
			float degrees = std::fmod(angle * 180.f / SDL_PI_F, 360.f);
			if (degrees < 0.0f) degrees += 360.f;
			SDL_Log("angle: %.1f degrees (%.3f radians)", degrees, angle);
		}

		SDL_SetRenderDrawColor(state.renderer, 30, 30, 40, 255);
		SDL_RenderClear(state.renderer);


		//work out the 4 corner of the square, rotated
		//corners are described around (0, 0), then rotated, then moved to the center.
		const SDL_FPoint corners[4] = { {-half, -half}, {half, -half}, {half, half}, {-half, half} };

		SDL_FPoint pts[5];
		for (int i = 0; i < 4; ++i)
		{
			pts[i].x = center.x + corners[i].x * c - corners[i].y * s;
			pts[i].y = center.y + corners[i].x * s + corners[i].y * c;
		}
		pts[4] = pts[0];                          //close the shape
		SDL_SetRenderDrawColor(state.renderer, 240, 240, 240, 255);
		SDL_RenderLines(state.renderer, pts, 5);


		SDL_SetRenderDrawColor(state.renderer, 230, 80, 80, 255);
		SDL_RenderLine(state.renderer, center.x, center.y, tip.x, tip.y);
		drawCircle(state.renderer, tip, flickRadius);

		
		//enemies
		SDL_SetRenderDrawColor(state.renderer, 240, 200, 60, 255);
		for (const auto& en : enemies)
		{
			SDL_FRect r = { en.pos.x - enemySize / 2.0f, en.pos.y - enemySize / 2.0f, enemySize, enemySize };
			SDL_RenderFillRect(state.renderer, &r);
		}
		SDL_RenderPresent(state.renderer);
	}

	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
	return 0;
}


void drawCircle(SDL_Renderer *r, SDL_FPoint center, float radius)
{
	const int N = 28;
	SDL_FPoint pts[N + 1];
	for (int i = 0; i <= N; ++i)
	{
		float a = (float)i / N * 2.f * SDL_PI_F;
		pts[i] = { center.x + std::cos(a) * radius, center.y + std::sin(a) * radius };
	}
	SDL_RenderLines(r, pts, N + 1);
}