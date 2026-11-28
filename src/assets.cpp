#include "assets.h"
#include "SDL3_image/SDL_image.h"
#include <string>
#include <vector>

//Textures are looked up in the RESOURCES folder cause I am lazy to make a separate file for stuff
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filepath)
{
	std::vector<SDL_Texture*> textures;
	//load game assets
	SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
	textures.push_back(tex);

	if (!tex)
		SDL_Log("Could not load this shit %s: %s", filepath.c_str(), SDL_GetError());
	else
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
	return tex;
}

bool Assets::load(SDL_Renderer* r)
{
	idleTex = loadTexture(r, RESOURCES_PATH "hand_frame_1.png");
	flickTex = loadTexture(r, RESOURCES_PATH "hand_frame_2.png");
	recoverTex = loadTexture(r, RESOURCES_PATH "hand_frame_3.png");
	mosquitoTex = loadTexture(r, RESOURCES_PATH "mosquito_frame.png");

	return idleTex && flickTex && recoverTex && mosquitoTex;
}

void Assets::destroy()
{
	std::vector<SDL_Texture*> textures;
	for (auto& t : textures)
	{
		SDL_DestroyTexture(t);
		t = nullptr;
	}

	SDL_DestroyTexture(mosquitoTex);
	mosquitoTex = nullptr;
}