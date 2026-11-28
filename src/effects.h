#pragma once
//for screen flash and shake screen effects
//I just want everything to have their own files, yeah I am OCD like that

#include <SDL3/SDL.h>

class Effect
{
public:
	void triggreFlash() { flashTimer = flashDuration(); }
	void triggerShake() { shakeTimer = shakeDuration(); }

	void update(float dt);

	float flash() const;				//1.0 right after a trigger, fades to 0
	SDL_Rect shakeViewport() const;		//the (shifted) viewport to draw the world in


private:
	static float flashDuration();
	static float shakeDuration();

	float flashTimer = 0.f;
	float shakeTimer = 0.f;
};