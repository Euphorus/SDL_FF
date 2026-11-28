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

int Player::frame() const
{
	if (flickTimer <= 0.f)
		return cfg::FrameIdle;
	
	float elapsed = cfg::FlickCooldown - flickTimer;
	if (elapsed < cfg::FlickPoseTime)
		return cfg::FrameFlick;
	if (elapsed < cfg::FlickPoseTime + cfg::RecoverPoseTime)
		return cfg::FrameRecover;

	return cfg::FrameIdle;
}

void Player::update(float dt, float turnDirection)
{
	angle += turnDirection * cfg::TurnSpeed * dt;
	if (flickTimer > 0.f) flickTimer -= dt;
}

void Player::draw(SDL_Renderer* r, const Assets& assets, bool debug) const
{
	SDL_Texture* tex = assets.idleTex;
	int f = frame();
	if (f == cfg::FrameFlick && assets.flickTex)
		tex = assets.flickTex;
	else if (f == cfg::FrameRecover && assets.recoverTex)
		tex = assets.recoverTex;

	if (tex)
	{
		float texW = 0.f, texH = 0.f;
		SDL_GetTextureSize(tex, &texW, &texH);

		//scale the frame so it is HandDrawnWidth wide on the canvas
		float scale = cfg::HandDrawnWidth / texW;
		float w = texW * scale, h = texH * scale;

		//the pivot, measured from the top-left of the drawn frame
		SDL_FPoint pivot = { cfg::HandPivotX * w, cfg::HandPivotY * h };

		//place the frame so its pivot lands exactly on the player's position
		SDL_FRect dst = { pos.x - pivot.x, pos.y - pivot.y, w, h };


		float elapsed = cfg::FlickCooldown - flickTimer;
			float t = elapsed / cfg::FlickPoseTime;
			//adding a little something something to help the player where to aim
			SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(r, 220, 60, 60, (Uint8)(220 * (1.f - t)));
			drawCircle(r, tip(), cfg::FlickRadius * (0.1f + 0.1f * t));
			SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
		

		//SDL rotates around 'pivot' and wants degrees
		SDL_RenderTextureRotated(r, tex, nullptr, &dst, angle * cfg::RadToDeg, &pivot, SDL_FLIP_NONE);

	}
}