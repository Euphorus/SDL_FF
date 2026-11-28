#pragma once
//game.h = Game Manager
//decides what happens each frame: events -> udpate -> render

#include <SDL3/SDL.h>
#include "player.h"
#include "enemy.h"
#include "effects.h"

enum class GameState { Playing, GameOver };

class Game
{
public: 
	bool init();
	void run();
	void shutdown();


private:
	void handleEvents(bool& flickPressed);
	void update(float dt, bool flickPressed);
	void rendertypeshi();
	void reset();

	SDL_Window* window = nullptr;
	SDL_Renderer* render = nullptr;
	bool running = false;

	GameState state = GameState::Playing;
	Player player;
	EnemyManager enemies;
	Effect effects;

	int health = cfg::StartHealth;
	int score = 0;
	float gameTime = 0.0f;   //seconds survived, drives the difficulty ramp
};