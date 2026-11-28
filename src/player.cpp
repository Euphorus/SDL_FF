#include "player.h"
#include "draw.h"
#include <cmath>

SDL_FPoint Player::tip() const
{
	return {
		pos.x + cfg::PointerLen * std::cos(angle),
		pos.y + cfg::PointerLen * std::sin(angle)
	};
}

void Player::update(float dt, float turnDirection)
{
	angle += turnDirection * cfg::TurnSpeed * dt;
	if (flickTimer > 0.f) flickTimer -= dt;
}

void Player::draw(SDL_Renderer* r) const
{
	SDL_FPoint t = tip();

	//the square body
	SDL_SetRenderDrawColor(r, 240, 240, 240, 255);
	drawRotatedSquare(r, pos, cfg::PlayerHalf * 2.f, angle);

	//the finger
	SDL_SetRenderDrawColor(r, 230, 80, 80, 255);
	SDL_RenderLine(r, pos.x, pos.y, t.x, t.y);

	//the flick zone: white while cooling down, red when ready
	if (canFlick())
		SDL_SetRenderDrawColor(r, 230, 80, 80, 255);
	else
		SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
	drawCircle(r, t, cfg::FlickRadius);
}