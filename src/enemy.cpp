#include "enemy.h"
#include "config.h"
#include "draw.h"
#include <algorithm>
#include <cmath>

//Pick a random spot just outside one of the 4 screen edges
SDL_FPoint randomSpawnPoints()
{
	const float margin = 30.f;
	switch (SDL_rand(4))
	{
	case 0:
		return { SDL_randf() * cfg::ScreenW, -margin };
		break;

	case 1:
		return { SDL_randf() * cfg::ScreenW, cfg::ScreenH + margin };
		break;

	case 2:
		return { -margin, SDL_randf() * cfg::ScreenH };
		break;

	default:
		return { cfg::ScreenW + margin, SDL_randf() * cfg::ScreenH };
		break;
	}
}

void EnemyManager::spawn()
{
	Enemy en;
	en.pos = randomSpawnPoints();
	enemies.push_back(en);
}

int EnemyManager::update(float dt, const Player& player, float gameTime)
{
	int bites = 0;

	//difficulty ramps up the longer you survive
	float spawnInterval = std::max(cfg::MinSpawnInterval, cfg::StartSpawnInterval - gameTime * cfg::SpawnRamp);
	float speed = std::min(cfg::MaxEnemySpeed, cfg::StartEnemySpeed + gameTime * cfg::SpeedRamp);

	spawnTimer -= dt;
	if (spawnTimer <= 0.f)
	{
		spawn();
		spawnTimer = spawnInterval;
	}

	for (auto& en : enemies)
	{
		//flicked enemies
		if (en.flicked)
		{
			en.pos.x += en.vel.x * dt;
			en.pos.y += en.vel.y * dt;
			en.angle += en.spin * dt;

			const float off = 80.f;
			if (en.pos.x < -off || en.pos.x > cfg::ScreenW + off ||
				en.pos.y < -off || en.pos.y > cfg::ScreenH + off)
				en.alive = false;
			continue;
		}

		//chasing enemies
		float dx = player.pos.x - en.pos.x;
		float dy = player.pos.y - en.pos.y;
		float dist = std::sqrt(dx * dx + dy * dy);

		en.angle = std::atan2(dy, dx);   //face the player

		if (dist > 0.0001f)
		{
			en.pos.x += (dx / dist) * speed * dt;
			en.pos.y += (dy / dist) * speed * dt;
		}

		//the bite: circle vs circle collision
		if (dist <= cfg::PlayerRadius + cfg::EnemyRadius)
		{
			//SDL_Log("HIT! an enemy touched the player.");
			bites++;
			en.alive = false;
		}
	}

	//remove dead enemies after the loop, never while looping
	enemies.erase(
		std::remove_if(enemies.begin(), enemies.end(),
			[](const Enemy& en) {return !en.alive; }),
		enemies.end());

	return bites;
}

int EnemyManager::flick(const Player& player)
{
	SDL_FPoint tip = player.tip();
	int count = 0;

	for (auto& en : enemies)
	{
		if (en.flicked) continue;

		float dx = tip.x - en.pos.x;
		float dy = tip.y - en.pos.y;
		float r = cfg::FlickRadius + cfg::EnemyRadius;
		if (dx * dx + dy * dy <= r * r)
		{
			//SDL_Log("FLICKED");
			en.flicked = true;
			en.vel = { std::cos(player.angle) * cfg::FlickSpeed,
					  std::sin(player.angle)* cfg::FlickSpeed };
			en.spin = cfg::FlickSpin;
			count++;		
		}
	}
	return count;
}

void EnemyManager::draw(SDL_Renderer* r, const Assets& assets, bool debug) const
{
	for (const auto& en : enemies)
	{
		if (assets.mosquitoTex)
		{
			//flicked mosquitoes are drawn a little see-through
			//SDL_SetTextureAlphaMod(assets.mosquitoTex, en.flicked ? 170 : 255);

			SDL_FRect dst = { en.pos.x - cfg::EnemySize / 2.f, en.pos.y - cfg::EnemySize / 2.f,
			cfg::EnemySize, cfg::EnemySize };

			SDL_RenderTextureRotated(r, assets.mosquitoTex, nullptr, &dst, en.angle * cfg::RadToDeg, nullptr, SDL_FLIP_NONE);
		}
		else
		{
			if (en.flicked)
				SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
			else
				SDL_SetRenderDrawColor(r, 240, 200, 60, 255);
			drawRotatedSquare(r, en.pos, cfg::EnemySize, en.angle);

		}		
	}

	if (assets.mosquitoTex)
		SDL_SetTextureAlphaMod(assets.mosquitoTex, 255);
}

void EnemyManager::clear()
{
	enemies.clear();
	spawnTimer = 0.f;
}