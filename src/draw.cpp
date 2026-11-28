#include "draw.h"
#include "config.h"
#include <cmath>

void drawCircle(SDL_Renderer* r, SDL_FPoint center, float radius)
{
	const int N = 28;
	SDL_FPoint pts[N + 1];
	for (int i = 0; i <= N; ++i)
	{
		float a = (float)i / N * 2.f * SDL_PI_F;
		pts[i] = { center.x + std::cos(a) * radius, center.y + std::sin(a) * radius };

	}
	SDL_RenderLines(r, pts, N + 1);
}

void drawRotatedSquare(SDL_Renderer* r, SDL_FPoint pos, float size, float angle)
{
	float h = size / 2.f, c = std::cos(angle), s = std::sin(angle);
	const SDL_FPoint corners[4] = { {-h, -h}, {h, -h}, {h, h}, {-h, h} };
	SDL_FPoint pts[5];
	for (int i = 0; i < 4; ++i)
	{
		pts[i].x = pos.x + corners[i].x * c - corners[i].y * s;
		pts[i].y = pos.y + corners[i].x * s + corners[i].y * c;
	}

	pts[4] = pts[0];
	SDL_RenderLines(r, pts, 5);
	SDL_RenderLine(r, pos.x, pos.y, pos.x + h * c, pos.y + h * s); 
}

void drawText(SDL_Renderer* r, float x, float y, float scale, const char* text)
{
	SDL_SetRenderScale(r, scale, scale);
	SDL_RenderDebugText(r, x / scale, y / scale, text);
	SDL_SetRenderScale(r, 1.f, 1.f);
}

void drawTextCentered(SDL_Renderer* r, float y, float scale, const char* text)
{
	float w = (float)SDL_strlen(text) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * scale;
	drawText(r, (cfg::ScreenW - w) / 2.f, y, scale, text);
}