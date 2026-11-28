#pragma once
//player.h - the rotating hand. Owns its own angles and flick cooldown

#include <SDL3/SDL.h>
#include "config.h"

struct Player
{
	SDL_FPoint pos{ cfg::ScreenW / 2.f, cfg::ScreenH / 2.f };
	float angle = 0.f;
	float flickTimer = 0.f;

	SDL_FPoint tip() const;    //where the fingertip (flick zone center) is
	bool canFlick() const { return flickTimer <= 0.f; }
	void startFlick() { flickTimer = cfg::FlickCooldown; }

	void update(float dt, float turnDirection);   //-1, 0, +1
	void draw(SDL_Renderer* r) const;
};