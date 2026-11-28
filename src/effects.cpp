#include "effects.h"
#include "config.h"
#include <algorithm>

float Effect::flashDuration() { return cfg::FlashDuration; }
float Effect::shakeDuration() { return cfg::ShakeDuration; }

void Effect::update(float dt)
{
	flashTimer = std::max(0.f, flashTimer - dt);
	shakeTimer = std::max(0.f, shakeTimer - dt);
}

float Effect::flash() const
{
	return flashTimer / cfg::FlashDuration;
}

SDL_Rect Effect::shakeViewport() const
{
	int ox = 0, oy = 0;
	if (shakeTimer > 0.f)
	{
		float k = shakeTimer / cfg::ShakeDuration;   //strong first, fades out
		ox = (int)((SDL_randf() * 2.f - 1.f) * cfg::ShakeStrength * k);
		oy = (int)((SDL_randf() * 2.f - 1.f) * cfg::ShakeStrength * k);
	}

	return { ox, oy, cfg::ScreenW, cfg::ScreenH };
}