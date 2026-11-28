#pragma once
//EnemyManager stuff for spawning, chasing, getting flicked and drawing

#include <SDL3/SDL.h>
#include <vector>
#include "player.h"

struct Enemy
{
	SDL_FPoint pos;
	SDL_FPoint vel{};
	float angle = 0.0f;
	float spin = 0.0f;
	bool flicked = false;
	bool alive = true;
};

class EnemyManager
{
public:
	//Moves everything
	int update(float dt, const Player& player, float gameTime);

	//flicks every chasing enemy inside the fingertip zone. returns how many were flicked
	int flick(const Player& player);

	void draw(SDL_Renderer* r) const;
	void clear();

private:
	void spawn();

	std::vector<Enemy> enemies;
	float spawnTimer = 0.0f;
};