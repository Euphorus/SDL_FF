#pragma once

#include <SDL3/SDL.h>

struct Assets
{
	//std::vector<SDL_Texture*> textures;
	SDL_Texture *idleTex, *recoverTex, *flickTex, *mosquitoTex;

	bool load(SDL_Renderer* r);
	void destroy();
};