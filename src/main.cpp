#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <array>
#include <format>

#include "gameObject.h"

struct SDLState
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logW, logH;
	const bool* keys;

	SDLState() : keys(SDL_GetKeyboardState(nullptr))
	{
	}
};

const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 50;
const int TILE_SIZE = 32;

struct GameState
{
	std::array<std::vector<GameObject>, 2> layers;
	std::vector<GameObject> backgroundTiles;
	std::vector<GameObject> foregroundTiles;
	std::vector<GameObject> bullets;

	int playerIndex;
	SDL_FRect mapViewport;
	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	GameState(const SDLState &state)
	{
		playerIndex = -1; //Will change this when we load map
		mapViewport = SDL_FRect{
			.x = 0, .y = 0, 
			.w = static_cast<float>(state.logW), .h = static_cast<float>(state.logH)
		};
		bg2Scroll = bg3Scroll = bg4Scroll = 0;
		debugMode = false;
	}

	GameObject& player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; }
};


struct Resources  //it will help load and unload all of the game assets, so textures, sound files, pointers, animations
{
	const int ANIM_PLAYER_IDLE = 0; //index of the idle animation
	const int ANIM_PLAYER_RUN = 1; //index for the run animation
	const int ANIM_PLAYER_SLIDE = 2; //index for the slide animation
	const int ANIM_PLAYER_SHOOT = 3;
	const int ANIM_PLAYER_SLIDE_SHOOT = 4;
	std::vector<Animation> playerAnimation;

	//animation integar for bullet animations
	const int ANIM_BULLET_MOVING = 0;
	const int ANIM_BULLET_HIT = 1;
	std::vector<Animation> bulletAnimation;

	//vector for enemy animations
	const int ANIM_ENEMY_IDLE = 0;
	const int ANIM_ENEMY_HIT = 1;
	const int ANIM_ENEMY_DIE = 2;
	std::vector<Animation> enemyAnimation;

	std::vector<SDL_Texture*> textures;
	SDL_Texture *idleTex, *runTex, *slideTex, *brickTex, *grassTex, *groundTex, *panelTex, 
		*bg1Tex, *bg2Tex, *bg3Tex, *bg4Tex, *bulletTex, *bulletHitTex,
		*texShoot, *texRunShoot, *texSlideShoot,
		*enemyTex, *enemyHitTex, *enemyDeadTex;

	SDL_Texture* loadTexture(SDL_Renderer *renderer, const std::string& filepath)
	{
		//load game assets
		SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	void load(SDLState& state) //This function will handle loading or unloading all game assets. WTF!!!
	{
		playerAnimation.resize(5);
		playerAnimation[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnimation[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnimation[ANIM_PLAYER_RUN] = Animation(4, 0.6f);
		playerAnimation[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
		playerAnimation[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);

		//loading the player movement related textures
		idleTex = loadTexture(state.renderer, RESOURCES_PATH "idle.png");
		runTex = loadTexture(state.renderer, RESOURCES_PATH "run.png");
		slideTex = loadTexture(state.renderer, RESOURCES_PATH "slide.png");
		texShoot = loadTexture(state.renderer, RESOURCES_PATH "shoot.png");
		texRunShoot = loadTexture(state.renderer, RESOURCES_PATH "shoot_run.png");
		texSlideShoot = loadTexture(state.renderer, RESOURCES_PATH "slide_shoot.png");

		//loading the ground-related textures
		brickTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/brick.png");
		grassTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/grass.png");
		groundTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/ground.png");
		panelTex = loadTexture(state.renderer, RESOURCES_PATH "tiles/panel.png");
		//loading the background textures
		bg1Tex = loadTexture(state.renderer, RESOURCES_PATH "bg/bg_layer1.png");
		bg2Tex = loadTexture(state.renderer, RESOURCES_PATH "bg/bg_layer2.png");
		bg3Tex = loadTexture(state.renderer, RESOURCES_PATH "bg/bg_layer3.png");
		bg4Tex = loadTexture(state.renderer, RESOURCES_PATH "bg/bg_layer4.png");

		//loading the bullet textures
		bulletAnimation.resize(2);
		bulletAnimation[ANIM_BULLET_MOVING] = Animation(4, 0.05f);
		bulletAnimation[ANIM_BULLET_HIT] = Animation(4, 0.1f);
		bulletTex = loadTexture(state.renderer, RESOURCES_PATH "bullet.png");
		bulletHitTex = loadTexture(state.renderer, RESOURCES_PATH "bullet_hit.png");

		enemyAnimation.resize(3);
		enemyAnimation[ANIM_ENEMY_IDLE] = Animation(8, 1.5f);
		enemyAnimation[ANIM_ENEMY_HIT] = Animation(8, 1.2f);
		enemyAnimation[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

		//loading the enemy textures
		enemyTex = loadTexture(state.renderer, RESOURCES_PATH "enemy.png");
		enemyHitTex = loadTexture(state.renderer, RESOURCES_PATH "enemy_hit.png");
		enemyDeadTex = loadTexture(state.renderer, RESOURCES_PATH "enemy_die.png");
	}

	void unload()
	{
		for (SDL_Texture* tex : textures)
		{
			SDL_DestroyTexture(tex);
		}
	}
};


bool initialize(SDLState& state);
void cleanup(SDLState& state);
void drawOjbect(const SDLState& state, GameState& gs, GameObject& obj, float width, float height, float deltaTime);
void update(SDLState& state, GameObject& obj, GameState& gs, Resources& res, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, const Resources& res);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltatime);
void handleKeyInput(const SDLState& state, GameState& gs, GameObject& obj, SDL_Scancode key, bool keyDown);
void drawParallaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime);

int main(int argc, char* argv[])
{
	SDLState state;

	state.width = 1280;
	state.height = 720;
	state.logW = 640;
	state.logH = 320;

	if (!initialize(state))
	{
		return 1;
	}

	//load game assets
	Resources res;
	res.load(state);

	//setup game data
	GameState gs(state);
	createTiles(state, gs, res);
	uint64_t prevTime = ((double)SDL_GetTicks());
	
	//start the gameloop
	bool running = true;
	while (running)
	{
		uint64_t nowTime = ((double)SDL_GetTicks()); //convert it from milliseconds to seconds.
		float deltaTime = (nowTime - prevTime) / 1000.f;
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_KEY_DOWN:
				handleKeyInput(state, gs, gs.player(), event.key.scancode, true);
				break;

			case SDL_EVENT_KEY_UP:
				handleKeyInput(state, gs, gs.player(), event.key.scancode, false);
				if (event.key.scancode == SDL_SCANCODE_P)
				{
					gs.debugMode = !gs.debugMode;
				}
				break;
			}
		}

		//update all objects
		for (auto& layer : gs.layers)
		{
			for (GameObject& obj : layer)
			{
				update(state, obj, gs, res, deltaTime);
			}
		}
		
		//update bullets 
		for (GameObject& bullet : gs.bullets)
		{
			update(state, bullet, gs, res, deltaTime);
		}

		//calculate viewport position
		gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;

		//perform drawing commands
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		//draw background images
		SDL_RenderTexture(state.renderer, res.bg1Tex, nullptr, nullptr);
		drawParallaxBackground(state.renderer, res.bg4Tex, gs.player().velocity.x, gs.bg4Scroll, 0.075f, deltaTime);
		drawParallaxBackground(state.renderer, res.bg3Tex, gs.player().velocity.x, gs.bg3Scroll, 0.15f, deltaTime);
		drawParallaxBackground(state.renderer, res.bg2Tex, gs.player().velocity.x, gs.bg2Scroll, 0.3f, deltaTime);

		//draw background tiles
		for (GameObject& obj : gs.backgroundTiles)
		{
			SDL_FRect dst{
				.x = obj.position.x - gs.mapViewport.x, 
				.y = obj.position.y,
				.w = static_cast<float>(obj.texture->w),
				.h = static_cast<float>(obj.texture->h)
			};
			SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
		}

		//draw all objects
		for (auto& layer : gs.layers)
		{
			for (GameObject& obj : layer)
			{
				drawOjbect(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
			}
		}

		//draw bullets
		for (GameObject& bullet : gs.bullets)
		{
			if (bullet.data.bullet.state != BulletState::inactive)
			{
				drawOjbect(state, gs, bullet, bullet.collider.w, bullet.collider.h, deltaTime);
			}
		}


		//draw foreground tiles
		for (GameObject& obj : gs.foregroundTiles)
		{
			SDL_FRect dst{
				.x = obj.position.x - gs.mapViewport.x, 
				.y = obj.position.y,
				.w = static_cast<float>(obj.texture->w),
				.h = static_cast<float>(obj.texture->h)
			};
			SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
		}

		if (gs.debugMode)
		{
			//display some debug info
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5, std::format("S: {}, B: {}, G: {}",
				static_cast<int>(gs.player().data.player.state), gs.bullets.size(), gs.player().grounded).c_str());

		}
		
		//swap the buffers
		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
	}

	

	res.unload();
	cleanup(state);
	return 0;
}

bool initialize(SDLState& state)
{
	bool initSuccess = true;

	//create the window
	state.window = SDL_CreateWindow("SDL is working", state.width, state.height, 0);
	if (!state.window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", state.window);
		cleanup(state);
		initSuccess = false;
	}

	//create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		initSuccess = false;
	}

	//configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	SDL_SetRenderVSync(state.renderer, 1);

	return initSuccess;
}

void cleanup(SDLState& state)
{
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}

void drawOjbect(const SDLState& state, GameState& gs, GameObject& obj, float width, float height ,float deltaTime)
{
	float srcX = obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame() * width : (obj.spriteFrame - 1) * width;

	SDL_FRect src{
		src.x = srcX,
		src.y = 0,
		src.w = width,
		src.h = height
	};

	SDL_FRect dst{
		dst.x = obj.position.x - gs.mapViewport.x,
		dst.y = obj.position.y,
		dst.w = width,
		dst.h = height
	};

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

	if (!obj.shouldFlash)
	{
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
	}
	else
	{
		//flash object with 
		SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.0f, 1.0f);
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
		SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);
		
		if (obj.flashTimer.step(deltaTime))
		{
			obj.shouldFlash = false;
		}
	}
	//SDL_RenderTexture(state.renderer, idleTex, &src, &dst);
	

	if (gs.debugMode)
	{
		SDL_FRect rectA{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y,
			.w = obj.collider.w,
			.h = obj.collider.h
		};

		SDL_FRect sensor{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y + obj.collider.h - gs.mapViewport.y,
			.w = obj.collider.w, .h = 1
		};

		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(state.renderer, 255, 45, 2, 150);
		SDL_RenderFillRect(state.renderer, &rectA);

		SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 150);
		SDL_RenderFillRect(state.renderer, &sensor);
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
	}
}

void update(SDLState& state, GameObject& obj, GameState& gs, Resources& res, float deltaTime)
{


	//update the animation
	if (obj.currentAnimation != -1)
	{
		obj.animations[obj.currentAnimation].step(deltaTime);
	}

	float currentDirection = 0;

	if (obj.dynamic && !obj.grounded)
	{
		//apply some gravity
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	}
	

	if (obj.type == ObjectType::player)
	{
		
		if (state.keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
		}
		if (state.keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
		}
		

		Timer& weaponTimer = obj.data.player.weaponTimer;
		weaponTimer.step(deltaTime);
		const auto handleShooting = [&state, &gs, &res, &obj, &weaponTimer](
			SDL_Texture *tex, SDL_Texture *shootTex, int animIndex, int shootAnimIndex)
		{
				if (state.keys[SDL_SCANCODE_J])
				{
					//set shooting tex/anim
					obj.texture = shootTex;
					obj.currentAnimation = shootAnimIndex;
					if (weaponTimer.isTimeout())
					{
						weaponTimer.reset();
						//spawn some bullets
						GameObject bullet;
						bullet.data.bullet = BulletData();
						bullet.type = ObjectType::bullet;
						bullet.direction = gs.player().direction;
						bullet.texture = res.bulletTex;
						bullet.currentAnimation = res.ANIM_BULLET_MOVING;
						bullet.collider = SDL_FRect{
							.x = 0, .y = 0,
							.w = static_cast<float>(res.bulletTex->h),
							.h = static_cast<float>(res.bulletTex->h)
						};
						const int yVariation = 50;
						const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
						bullet.velocity = glm::vec2(
							obj.velocity.x + 600.f * obj.direction, yVelocity
						);
						bullet.maxSpeedX = 1000.0f;
						bullet.animations = res.bulletAnimation;

						//adjust bullet starting position
						const float left = 4;
						const float right = 24;
						const float t = (obj.direction + 1) / 2.0f; //result in a value of 0..1
						const float xOffset = left + right * t; //LERP between left and right based on direciton
						bullet.position = glm::vec2(
							obj.position.x + xOffset,
							obj.position.y + TILE_SIZE / 2 + 1
						);

						//look for inactive bullets
						bool foundInactive = false;
						for (int i = 0; i < gs.bullets.size() && !foundInactive; i++)
						{
							if (gs.bullets[i].data.bullet.state == BulletState::inactive)
							{
								foundInactive = true;
								gs.bullets[i] = bullet;
							}
						}
						if (!foundInactive)
						{
							gs.bullets.push_back(bullet);
						}		
					}
				}
				else
				{
					obj.texture = tex;
					obj.currentAnimation = animIndex;
				}
		};

		switch (obj.data.player.state)
		{
		case PlayerState::idle:
			//switching to run state
			if (currentDirection)
			{
				obj.data.player.state = PlayerState::running;
			}
			else
			{
				//decelerate
				if (obj.velocity.x)
				{
					const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
					float amount = factor * obj.acceleration.x * deltaTime;
					if (std::abs(obj.velocity.x) < std::abs(amount))
					{
						obj.velocity.x = 0;
					}
					else
					{
						obj.velocity.x += amount;
					}
				}
			}
			handleShooting(res.idleTex, res.texShoot, res.ANIM_PLAYER_IDLE, res.ANIM_PLAYER_SHOOT);
			break;

		case PlayerState::running:
			//switching to idle state
			if (!currentDirection)
			{
				obj.data.player.state = PlayerState::idle;
			}

			//handleShooting(res.runTex, res.texRunShoot, res.ANIM_PLAYER_IDLE, res.ANIM_PLAYER_SHOOT);

			//moving in opposite direction of velocity, sliding!!
			if (obj.velocity.x * obj.direction < 0 && obj.grounded)
			{
				handleShooting(res.slideTex, res.texSlideShoot, res.ANIM_PLAYER_SLIDE, res.ANIM_PLAYER_SLIDE_SHOOT);
			}
			else
			{
				handleShooting(res.runTex, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
			}
			break;

		case PlayerState::jumping:
			handleShooting(res.runTex, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
			break;
		}
		
	}
	else if (obj.type == ObjectType::bullet)
	{
		switch (obj.data.bullet.state)
		{
			case BulletState::moving:
			{
				if (obj.position.x - gs.mapViewport.x < 0 ||
					obj.position.x - gs.mapViewport.x > state.logW ||
					obj.position.y - gs.mapViewport.y < 0 ||
					obj.position.y - gs.mapViewport.y > state.logH)
				{
					obj.data.bullet.state = BulletState::inactive;
				}
				break;
			}

			case BulletState::collider:
			{
				if (obj.animations[obj.currentAnimation].isDone())
				{
					obj.data.bullet.state = BulletState::inactive;
				}
				break;
			}
		}
		
	}
	else if (obj.type == ObjectType::enemy)
	{
		switch (obj.data.enemy.state)
		{
			case EnemyState::shambling:
			{
				glm::vec2 playerDir = gs.player().position - obj.position;
				if (glm::length(playerDir) < 100)
				{
					currentDirection = playerDir.x < 0 ? -1 : 1;
					obj.acceleration = glm::vec2(30, 0);
				}
				else
				{
					obj.acceleration = glm::vec2(0);
					obj.velocity.x = 0;
				}
				break;
			}

			case EnemyState::damaged:
			{
				if (obj.data.enemy.damageTimer.step(deltaTime))
				{
					obj.data.enemy.state = EnemyState::shambling;
					obj.texture = res.enemyTex;
					obj.currentAnimation = res.ANIM_ENEMY_IDLE;
				}
				break;
			}

			case EnemyState::dead:
			{
				obj.velocity.x = 0;
				if (obj.currentAnimation != -1 && obj.animations[obj.currentAnimation].isDone())
				{
					obj.currentAnimation = -1;
					obj.spriteFrame = 18;
				}
				break;
			}
		}
	}

	if (currentDirection)
	{
		obj.direction = currentDirection;
	}

	//add acceleration to velocity
	obj.velocity += currentDirection * obj.acceleration * deltaTime;
	if (std::abs(obj.velocity.x) > obj.maxSpeedX)
	{
		obj.velocity.x = currentDirection * obj.maxSpeedX;
	}

	//add velocity to position
	obj.position += obj.velocity * deltaTime;

	//handle collision detection
	bool foundGround = false;
	for (auto& layer : gs.layers)
	{
		for (GameObject& objB : layer)
		{
			if (&obj != &objB)
			{
				checkCollision(state, gs, res, obj, objB, deltaTime);

				if (objB.type == ObjectType::level)
				{
					//Ground check (Unity ahh PTSD!)
					SDL_FRect sensor{
						.x = obj.position.x + obj.collider.x,
						.y = obj.position.y + obj.collider.y,
						.w = obj.collider.w, .h = obj.collider.h
					};

					SDL_FRect rectB{
						.x = objB.position.x + objB.collider.x,
						.y = objB.position.y + objB.collider.y,
						.w = objB.collider.w, .h = objB.collider.h
					};

					SDL_FRect rectC{ 0 };
					if (SDL_GetRectIntersectionFloat(&sensor, &rectB, &rectC))
					{
						foundGround = true;
					}
				}	
			}
		}
	}
	if (obj.grounded != foundGround)
	{
		//switching grounded state
		obj.grounded = foundGround;
		if (foundGround && obj.type == ObjectType::player)
		{
			obj.data.player.state = PlayerState::running;
		}
	}
}

void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& rectC, GameObject &objA, GameObject &objB, float deltaTime)
{
	const auto genericResponse = [&] ()
	{
		if (rectC.w < rectC.h)
		{
			//horizontal collision
			if (objA.velocity.x > 0) // going right
			{
				objA.position.x -= rectC.w;
			}
			else if (objA.velocity.x < 0) //going left
			{
				objA.position.x += rectC.w;
			}
			objA.velocity.x = 0;
		}
		else
		{
			//vertical collision
			if (objA.velocity.y > 0) // going down
			{
				objA.position.y -= rectC.h;
			}
			else if (objA.velocity.y < 0) // going up
			{
				objA.position.y += rectC.h;
			}
			objA.velocity.y = 0;
		}
	};

	//object we are checking
	if (objA.type == ObjectType::player)
	{
		//object it is colliding with
		switch (objB.type)
		{
			case ObjectType::level:
			{
				genericResponse();
				break;
			}
			case ObjectType::enemy:
			{
				if (objB.data.enemy.state != EnemyState::dead)
				{
					objA.velocity = glm::vec2(100, 0) * -objA.direction;
					//make the player take damage during collision
				}	
				break;
			}
		}
	}
	else if (objA.type == ObjectType::bullet)
	{
		bool passthrough = false;
		switch (objA.data.bullet.state)
		{
			case BulletState::moving:
			{
				switch (objB.type)
				{
					case ObjectType::level:
					{
						break;
					}

					case ObjectType::enemy:
					{
						EnemyData& d = objB.data.enemy;
						if (d.state != EnemyState::dead)
						{
							objB.direction = -objA.direction;
							objB.shouldFlash = true;
							objB.flashTimer.reset();
							objB.texture = res.enemyHitTex;
							objB.currentAnimation = res.ANIM_ENEMY_HIT;
							d.state = EnemyState::damaged;
							d.health -= 10;
							if (d.health <= 0)
							{
								d.state = EnemyState::dead;
								objB.texture = res.enemyDeadTex;
								objB.currentAnimation = res.ANIM_ENEMY_DIE;
							}
						}
						else
						{
							//don't collide with dead enemies
							passthrough = true;
						}
						break;
					}
				}
				if (!passthrough)
				{
					genericResponse();
					objA.velocity *= 0;
					objA.data.bullet.state = BulletState::collider;
					objA.texture = res.bulletHitTex;
					objA.currentAnimation = res.ANIM_BULLET_HIT;
				}	
				break;
			}

			case BulletState::collider:
			{
				break;
			}

			case BulletState::inactive:
			{
				break;
			}
		}
	}
	else if (objA.type == ObjectType::enemy)
	{
		genericResponse();
	}
}

//my favourite part of pure programming in programming games
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltatime)
{
	SDL_FRect rectA{
		.x = a.position.x + a.collider.x, 
		.y = a.position.y + a.collider.y,
		.w = a.collider.w, 
		.h = a.collider.h
	};

	SDL_FRect rectB{
		.x = b.position.x + b.collider.x,
		.y = b.position.y + b.collider.y,
		.w = b.collider.w,
		.h = b.collider.h
	};

	SDL_FRect rectC{ 0 };

	if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC))
	{
		//found intersection, respond accordingly
		collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltatime);
	}
}

void createTiles(const SDLState& state, GameState& gs, const Resources& res)
{
	/*
	* 0 - Empty Tiles
	* 1 - Ground
	* 2 - Panel
	* 3 - Enemy
	* 4 - Player
	* 5 - Grass
	* 6 - Brick
	*/

	short map[MAP_ROWS][MAP_COLS] = {
		0, 4, 0, 0, 0, 0, 0, 3, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 2, 3, 3, 3, 0, 2, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	short foreground[MAP_ROWS][MAP_COLS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 0, 0, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	short background[MAP_ROWS][MAP_COLS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	const auto loadMap = [&state, &gs, &res](short layer[MAP_ROWS][MAP_COLS])
	{
			const auto createObject = [&state](int r, int c, SDL_Texture* tex, ObjectType type)
				{
					GameObject o;
					o.type = type;
					o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
					o.texture = tex;
					o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE };
					return o;
				};
			for (int r = 0; r < MAP_ROWS; r++)
			{
				for (int c = 0; c < MAP_COLS; c++)
				{
					switch (layer[r][c])
					{
					case 1:
					{
						//Create the "ground" part of the level
						GameObject ground = createObject(r, c, res.groundTex, ObjectType::level);
						gs.layers[LAYER_IDX_LEVEL].push_back(ground);
						break;
					}

					case 2:
					{
						//Create the "panel" part of the level
						GameObject panel = createObject(r, c, res.panelTex, ObjectType::level);
						gs.layers[LAYER_IDX_LEVEL].push_back(panel);
						break;
					}

					case 3:
					{
						//Create our enemy in the game
						GameObject enemy = createObject(r, c, res.enemyTex, ObjectType::enemy);
						enemy.data.enemy = EnemyData();
						enemy.currentAnimation = res.ANIM_ENEMY_IDLE;
						enemy.animations = res.enemyAnimation;
						enemy.collider =
						{
							.x = 11, .y = 6,
							.w = 10, .h = 26
						};
						enemy.maxSpeedX = 15;
						enemy.dynamic = true;
						gs.layers[LAYER_IDX_CHARACTERS].push_back(enemy);
						break;
					}


					case 4: //players
					{
						//Create our player
						GameObject player = createObject(r, c, res.idleTex, ObjectType::player);
						player.data.player = PlayerData();
						player.texture = res.idleTex;
						player.animations = res.playerAnimation;
						player.currentAnimation = res.ANIM_PLAYER_IDLE;
						player.acceleration = glm::vec2(300, 0);
						player.maxSpeedX = 100;
						player.dynamic = true;
						player.collider = 
						{
							.x = 11, .y = 6,
							.w = 10, .h = 26
						};
						gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
						gs.playerIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;
						break;
					}

					case 5:
					{
						//Create the "grass" part of the level
						GameObject grass = createObject(r, c, res.grassTex, ObjectType::level);
						gs.foregroundTiles.push_back(grass);
						break;
					}

					case 6:
					{
						//Create the "brick" part of the level
						GameObject brick = createObject(r, c, res.brickTex, ObjectType::level);
						gs.backgroundTiles.push_back(brick);
						break;
					}
					}
				}
			}
	};
	loadMap(map);
	loadMap(background);
	loadMap(foreground);
	assert(gs.playerIndex != -1);
}

void handleKeyInput(const SDLState &state, GameState &gs, GameObject &obj, SDL_Scancode key, bool keyDown)
{
	const float JUMP_FORCE = -200.0f;
	if (obj.type == ObjectType::player)
	{
		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				if (key == SDL_SCANCODE_SPACE && keyDown)
				{
					obj.data.player.state = PlayerState::jumping;
					obj.velocity.y += JUMP_FORCE;
				}
				break;
			}
			case PlayerState::running:
			{
				if (key == SDL_SCANCODE_SPACE && keyDown)
				{
					obj.data.player.state = PlayerState::jumping;
					obj.velocity.y += JUMP_FORCE;
				}
				break;
			}
		}
	}
}

void drawParallaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime)
{
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w)
	{
		scrollPos = 0;
	}

	SDL_FRect dst{
		.x = scrollPos, .y = 30,
		.w = texture->w * 2.0f,
		.h = static_cast<float>(texture->h)
	};

	SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dst);
}