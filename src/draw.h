#pragma once
//draw.h - small drawing helpers. Just for drawing stuff

#include "SDL3/SDL.h"

void drawCircle(SDL_Renderer* r, SDL_FPoint center, float radius);
void drawRotatedSquare(SDL_Renderer* r, SDL_FPoint pos, float size, float angle);
void drawText(SDL_Renderer* r, float x, float y, float scale, const char* text);
void drawTextCentered(SDL_Renderer* r, float y, float scale, const char* text);
