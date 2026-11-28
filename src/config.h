#pragma once

//config.h - every tweakable number lives here
//want the game easier/harder/juicier? change the values in this file

#include <SDL3/SDL.h>

namespace cfg
{
	inline constexpr float DegToRad = SDL_PI_F / 180.f;

	//window 
	inline constexpr int ScreenW = 960;
	inline constexpr int ScreenH = 480;

	//player (Za Hando)
	inline constexpr float PlayerHalf   = 40.f; 
	inline constexpr float PlayerRadius = PlayerHalf;
	inline constexpr float TurnSpeed = 70.f * DegToRad;
	inline constexpr float PointerLen = 90.f; //center -> fingertip
	inline constexpr float FlickRadius = 35.f; //flick zone size
	inline constexpr float FlickCooldown = 0.8f;  //seconds


	//enemies
	inline constexpr float EnemySize = 24.f;
	inline constexpr float EnemyRadius = EnemySize / 2.f;
	inline constexpr float StartEnemySpeed = 60.f;
	inline constexpr float MaxEnemySpeed = 130.f;
	inline constexpr float SpeedRamp = 1.2f;
	inline constexpr float StartSpawnInterval = 2.0f;
	inline constexpr float MinSpawnInterval = 0.6f;
	inline constexpr float SpawnRamp = 0.025f;

	//flick feature
	inline constexpr float FlickSpeed = 600.f;
	inline constexpr float FlickSpin = 20.f;  //radians per second

	//game
	inline constexpr int StartHealth = 5;

	//juiciness
	inline constexpr float FlashDuration = 0.3f;
	inline constexpr float ShakeDuration = 0.15f;
	inline constexpr float ShakeStrength = 8.0f;
}