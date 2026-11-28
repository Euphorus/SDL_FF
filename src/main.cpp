#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cmath>
#include <vector>

#define SCR_WIDTH 640
#define SCR_HEIGHT 480


struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
};

struct Enemy {
	SDL_FPoint pos;
	SDL_FPoint vel{};
	float angle = 0.0f;
	float spin = 0.0f;
	bool flicked = false;
	bool alive = true;
};

const float flickSpeed = 600.f;
const float flickSpin = 20.f;
const int startHealth = 5;
int health = startHealth;
int score = 0;
bool gameOver = false;

float flashTimer = 0.f;
const float flashDuration = 0.3f; //how long the red flash lasts

float shakeTimer = 0.f;
const float shakeDuration = 0.15f; //how long the shake lasts
const float shakeStrength = 8.f;   //max shake distance in pixels

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
void drawRotatedSquare(SDL_Renderer* r, SDL_FPoint pos, float size, float angle);
void drawText(SDL_Renderer* r, float x, float y, float scale, const char* text);
void drawTextCentered(SDL_Renderer* r, float y, float scale, const char* text);

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
	const float turnSpeed = 70.f * SDL_PI_F / 180.f;
	float angle = 0;
	const float pointerLen = 90.f;
	const float flickRadius = 35.f;
	const float flickCooldown = 0.8f;
	float flickTimer = 0.0f;
	

	//enemy (the something something something)
	std::vector<Enemy> enemies;
	const float enemySize = 24.f;
	const float enemyRadius = enemySize / 2.0f;
	const float enemySpeed = 60.f;
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

		bool flickPress = false;

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
						flickPress = true;
					break;
			}

		}

		//create placeholder inputs for testing purposes
		const bool* keys = SDL_GetKeyboardState(nullptr);
		float dir = 0.0f;
		if (keys[SDL_SCANCODE_A]) dir -= 1.f;
		if (keys[SDL_SCANCODE_D]) dir += 1.f;
		

		//ENEMY MOVE LOOP
		//---------------------------
		angle += dir * turnSpeed * deltaTime;
		float c = std::cos(angle), s = std::sin(angle);
		SDL_FPoint tip = { center.x + pointerLen * c, center.y + pointerLen * s };

		if (gameOver)
		{
			if (flickPress)
			{
				health = startHealth;
				score = 0;
				enemies.clear();
				spawnTimer = 0.f;
				flickTimer = 0.f;
				gameOver = false;
			}
		}
		else
		{
			if (flashTimer > 0.f)
				flashTimer = std::max(0.f, flashTimer - deltaTime);

			if (shakeTimer > 0.f)
				shakeTimer = std::max(0.f, shakeTimer - deltaTime);

			if (flickTimer > 0.f) flickTimer -= deltaTime;
			if (flickPress && flickTimer <= 0.0f)
			{
				flickTimer = flickCooldown;
				for (auto& en : enemies)
				{
					//handle flicked enemies first
					if (en.flicked)
					{
						en.pos.x += en.vel.x * deltaTime;
						en.pos.y += en.vel.y * deltaTime;
						en.angle += en.spin * deltaTime;

						const float off = 80.f;
						if (en.pos.x < -off || en.pos.x > SCR_WIDTH + off || en.pos.y < -off || en.pos.y > SCR_HEIGHT + off)
							en.alive = false;
						continue;
					}

					float dx = tip.x - en.pos.x;
					float dy = tip.y - en.pos.y;
					float r = flickRadius + enemyRadius;
					if (dx * dx + dy * dy <= r * r)
					{
						SDL_Log("FLICKED");
						score++;
						shakeTimer = shakeDuration;
						en.flicked = true;
						en.vel = { std::cos(angle) * flickSpeed, std::sin(angle) * flickSpeed };
						en.spin = flickSpin;
					}

				}

			}

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

				//flicked enemies: fly away and tumble, no chasing, no biting
				if (en.flicked)
				{
					en.pos.x += en.vel.x * deltaTime;
					en.pos.y += en.vel.y * deltaTime;
					en.angle += en.spin * deltaTime;

					const float off = 80.f;
					if (en.pos.x < -off || en.pos.x > SCR_WIDTH + off ||
						en.pos.y < -off || en.pos.y > SCR_HEIGHT + off)
						en.alive = false;
					continue;
				}

				//direction = (target - position), then shrink it to length 1
				float dx = center.x - en.pos.x;
				float dy = center.y - en.pos.y;
				float dist = std::sqrt(dx * dx + dy * dy);

				en.angle = std::atan2(dy, dx);

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
					health--;
					flashTimer = flashDuration;
					en.alive = false;
				}
			}

			//remove enemies that hit (done after the loop we don't erase while loading
			enemies.erase(
				std::remove_if(enemies.begin(), enemies.end(),
					[](const Enemy& en) {return !en.alive; }),
				enemies.end()
			);

			if (health <= 0) gameOver = true;
		}
		

		////debug log for checking the angle
		//if (dir != 0.0f)
		//{
		//	float degrees = std::fmod(angle * 180.f / SDL_PI_F, 360.f);
		//	if (degrees < 0.0f) degrees += 360.f;
		//	SDL_Log("angle: %.1f degrees (%.3f radians)", degrees, angle);
		//}

		float f = flashTimer / flashDuration;
		SDL_SetRenderDrawColor(state.renderer, (Uint8)(30 + 130 * f), 30, 40, 255);
		SDL_RenderClear(state.renderer);

		//adding the thug shake in the game
		float shakeX = 0.f, shakeY = 0.f;
		if (shakeTimer > 0.f)
		{
			float k = shakeTimer / shakeDuration;
			shakeX = (SDL_randf() * 2.f - 1.f) * shakeStrength * k;
			shakeY = (SDL_randf() * 2.f - 1.f) * shakeStrength * k;
		}
		SDL_Rect view = { (int)shakeX, (int)shakeY, SCR_WIDTH, SCR_HEIGHT };
		SDL_SetRenderViewport(state.renderer, &view);

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


		if (flickTimer > 0.0f)	SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		else					SDL_SetRenderDrawColor(state.renderer, 230, 80, 80, 255);
		drawCircle(state.renderer, tip, flickRadius);

		//tip of the finger
		SDL_SetRenderDrawColor(state.renderer, 230, 80, 80, 255);
		SDL_RenderLine(state.renderer, center.x, center.y, tip.x, tip.y);

		SDL_SetRenderViewport(state.renderer, nullptr);
		
		//enemies
		SDL_SetRenderDrawColor(state.renderer, 240, 200, 60, 255);
		for (const auto& en : enemies)
		{
			if (en.flicked)
				SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			else
				SDL_SetRenderDrawColor(state.renderer, 240, 200, 60, 255);
			drawRotatedSquare(state.renderer, en.pos, enemySize, en.angle);
		}

		//score
		char buf[64];
		SDL_SetRenderDrawColor(state.renderer, 240, 240, 240, 255);
		SDL_snprintf(buf, sizeof(buf), "Score: %d", score);
		drawText(state.renderer, 12, 12, 2.f, buf);

		//hearts(one square per health point)
		for (int i = 0; i < health; ++i)
		{
			SDL_FRect heart = { SCR_WIDTH - 30.f - i * 28.f, 14.f, 20.f, 20.f };
			SDL_SetRenderDrawColor(state.renderer, 210, 50, 60, 255);
			SDL_RenderFillRect(state.renderer, &heart);
		}

		if (gameOver)
		{
			SDL_SetRenderDrawColor(state.renderer, 240, 240, 240, 255);
			drawTextCentered(state.renderer, 160, 4.f, "GAME OVER");
			drawTextCentered(state.renderer, 230, 2.f, "Press SPACE to restart");
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

void drawRotatedSquare(SDL_Renderer* r, SDL_FPoint pos, float size, float angle)
{
	float h = size / 2.f, c = std::cos(angle), s = std::sin(angle);
	const SDL_FPoint corners[4] = { {-h, -h}, {h, -h}, {h, h}, {-h, h} };
	SDL_FPoint pts[5];
	for (int i = 0; i < 4; i++)
	{
		pts[i].x = pos.x + corners[i].x * c - corners[i].y * s;
		pts[i].y = pos.y + corners[i].x * s + corners[i].y * c;
	}
	pts[4] = pts[0];
	SDL_RenderLines(r, pts, 5);
	SDL_RenderLine(r, pos.x, pos.y, pos.x + h * c, pos.y + h * s);
}

void drawText(SDL_Renderer* r, float x, float y, float scale, const char* text)
{
	SDL_SetRenderScale(r, scale, scale);
	SDL_RenderDebugText(r, x / scale, y / scale, text);
	SDL_SetRenderScale(r, 1.f, 1.f);
}

void drawTextCentered(SDL_Renderer* r, float y, float scale, const char* text)
{
	float w = (float)SDL_strlen(text) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * scale;
	drawText(r, (SCR_WIDTH - w) / 2.f, y, scale, text);
}