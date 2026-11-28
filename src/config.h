#pragma once

//config.h - every tweakable number lives here
//want the game easier/harder/juicier? change the values in this file

#include <SDL3/SDL.h>

namespace cfg
{
	inline constexpr float DegToRad = SDL_PI_F / 180.f;
	inline constexpr float RadToDeg = 180.f / SDL_PI_F;

	//window 
	inline constexpr int ScreenW = 720;
	inline constexpr int ScreenH = 720;

	//player (Za Hando)
	inline constexpr float PlayerHalf   = 40.f; 
	inline constexpr float PlayerRadius = PlayerHalf;
	inline constexpr float TurnSpeed = 95.f * DegToRad;
	inline constexpr float PointerLen = 120.f; //center -> fingertip
	inline constexpr float FlickRadius = 50.f; //flick zone size
	inline constexpr float FlickCooldown = 0.8f;  //seconds

	//---hand sprite related configs provided by dead Claudia
	inline constexpr float HandDrawnWidth = 320.f;
	inline constexpr float HandPivotX = 0.45f;
	inline constexpr float HandPivotY = 0.50f;

	//0 - hand_frame_1, 1 - hand_frame_2, 2 - hand_frame_3
	inline constexpr int FrameIdle = 0;
	inline constexpr int FrameFlick = 1;
	inline constexpr int FrameRecover = 2;
	inline constexpr float FlickPoseTime = 0.1f;
	inline constexpr float RecoverPoseTime = 0.35f;

	//enemies
	inline constexpr float EnemySize = 67.f;
	inline constexpr float EnemyRadius = EnemySize/2.f;
	inline constexpr float StartEnemySpeed = 60.f;
	inline constexpr float MaxEnemySpeed = 130.f;
	inline constexpr float SpeedRamp = 1.f;
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