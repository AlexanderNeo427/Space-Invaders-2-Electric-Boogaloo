#include "SceneAsteroid.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneAsteroid::SceneAsteroid()
{
}

SceneAsteroid::~SceneAsteroid()
{
}

void SceneAsteroid::Init()
{
	SceneBase::Init();

	// Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	// Physics code here
	m_speed = 1.f;
	m_torque.SetZero();
	
	Math::InitRNG();

	// Init m_goList
	for (int i = 0; i < 180; ++i)
		m_goList.push_back(new GameObject);

	SpawnGO(GameObject::GO_ASTEROID, CurrWave * 2);	    // Asteroid Init
	SpawnGO(GameObject::GO_ENEMY_SHIP, CurrWave * 2);	// Enemy Init
	SpawnGO(GameObject::GO_BUDDY, 1);				// Buddy ship Init

	m_lives = 3;
	m_score = 0;

	// Ship Init
	m_ship = NULL;
	m_ship = new GameObject(GameObject::GO_SHIP);
	m_ship->active = true;
	m_ship->scale.Set(6.1f, 5.3f, 1.f);
	m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0);
	m_ship->dir.Set(1, 0, 0);
	m_ship->momentOfInertia = ( m_ship->mass * m_ship->scale.x * m_ship->scale.x );	// Moment of Inertia: I = MR^2
	m_ship->angularVelocity = 0;	
	m_ship->HP = SHIP_HP;

	//// CUSTOM STUFFS
	// Weapon Section
	RoundsPerShot = 0;
	PenPow = 0;
	Spread = 0;
	bTime_Weapon = 0;
	bTime_Switch = 0;

	AsteroidRandTime = 0, EnemyRandTime = 0, pwrUpRandTime = 15;

	// Misc
	GameEnd = false;
	DT = 0;		
	b_RepelActive = b_RepelActiveTime = false;
	RepelTime = 3.f;
	Repel_bTime = 0;
	RepelCooldown = 42.f;	// Larger than cooldown time
	bTime_Buddy = 1.5f;
	WaveTime = ROUND_TIME;
	Z = 0;
	CurrWave = 1;
	Weapon = WEAPON_TYPE::RIFLE;

	NumActive = 0, NumEnemy = 0, NumAsteroid = 0;

	BlackHole_RandTime = Math::RandFloatMinMax(15, 25);

	x = y = 0;
	CurrMouseX = CurrMouseY = 0;

	ShowCursor(false);
}

GameObject* SceneAsteroid::FetchGO()
{
	for (std::vector<GameObject*>::iterator itr = m_goList.begin(); itr != m_goList.end(); ++itr)
	{
		GameObject* go = (GameObject*)*itr;

		if ( (itr + 1) == m_goList.end() )	// If at end of list && No objects available, 
		{
			if (go->active == false) return go;
			else 
			{
				for (int i = 0; i < 15; ++i)	// Create another 15 game objects
					m_goList.push_back(new GameObject);

				return FetchGO();
			}
		}
		else 
			if (go->active == false) return go;
	}

	return NULL;
}

void SceneAsteroid::Update(double dt)
{
	SceneBase::Update(dt);

	DT = static_cast<float>(dt);

	// Calculating mouse world pos
	Application::GetCursorPos(&x, &y);

	CurrMouseX = (x / Application::GetWindowWidth()) * m_worldWidth;
	CurrMouseY = (Application::GetWindowHeight() - y) / (Application::GetWindowHeight() / m_worldHeight);

	//  Calculate Aspect Ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//if (Application::IsKeyPressed('P'))		// Decrease Speed
	//	m_speed = Math::Min(10.f, m_speed + 2.f * DT);
	//if (Application::IsKeyPressed('O'))		// Increase Speed
	//	m_speed = Math::Max(0.f, m_speed - 2.f * DT);

	//=================================================================================================
	//										INPUT SECTION
	//=================================================================================================
	// Movement / Control
	m_force.SetZero();
	m_torque.SetZero();

	if (Application::IsKeyPressed('W'))  // Foward
	{
		m_force += 125 * m_ship->dir;
	}
	if (Application::IsKeyPressed('A'))  // Left
	{
		m_force += 15 * m_ship->dir;
		m_torque += Vector3(-m_ship->scale.x, -m_ship->scale.y, 0.f).Cross(Vector3(5.f, 0.f, 0.f));
	}
	if (Application::IsKeyPressed('S'))	 // Down
	{
		m_force -= 80 * m_ship->dir;
	}
	if (Application::IsKeyPressed('D'))	 // Right
	{
		m_force += 15 * m_ship->dir;
		m_torque += Vector3(-m_ship->scale.x, m_ship->scale.y, 0.f).Cross(Vector3(5.f, 0.f, 0.f));
	}

	// Changing Mass of Ship
	//if (Application::IsKeyPressed(VK_OEM_PLUS))		// Increase mass
	//{
	//	m_ship->mass += 2.f * DT;
	//	m_ship->momentOfInertia = (m_ship->mass * m_ship->scale.x * m_ship->scale.x);
	//}
	//if (Application::IsKeyPressed(VK_OEM_MINUS))	// Decrease mass
	//{
	//	m_ship->mass = Math::Max(Math::EPSILON, m_ship->mass - 2.f * DT);
	//	m_ship->momentOfInertia = (m_ship->mass * m_ship->scale.x * m_ship->scale.x);
	//}

	// Cycle through waves
	if (Application::IsKeyPressed(VK_OEM_PLUS))		// Next Wave
	{
		if (bounceTime > 0.7f)
		{
			++CurrWave;

			bounceTime = 0;
		}
	}
	if (Application::IsKeyPressed(VK_OEM_MINUS))	// Previous Wave
	{
		if (bounceTime > 0.7f)
		{
			--CurrWave;

			bounceTime = 0;
		}
	}

	// Gun Properties
	SetWeapon(Weapon);

	// Cycle through weapons
	if (Application::IsKeyPressed('E'))
	{
		if (bTime_Switch > 0.4f)
		{
			Weapon = static_cast<WEAPON_TYPE>((Weapon + 1) % NUM_WEAPON_TYPES);

			bTime_Switch = 0;
		}
	}

	if (Application::IsKeyPressed('Q'))
	{
		if (bTime_Switch > 0.4f)
		{
			if (Weapon == 0) 
				Weapon = static_cast<WEAPON_TYPE>(NUM_WEAPON_TYPES - 1);
			else 
				Weapon = static_cast<WEAPON_TYPE>((Weapon - 1) % NUM_WEAPON_TYPES);

			bTime_Switch = 0;
		}
	}

	// Spawn Powerup
	if (Application::IsKeyPressed('P') && bounceTime > 0.3)
	{
		SpawnGO(GameObject::GO_POWERUP, 1);
		bounceTime = 0;
	}

	// Spawn Buddy ship
	if (Application::IsKeyPressed('B') && bounceTime > 1)
	{
		SpawnGO(GameObject::GO_BUDDY, 1);
		bounceTime = 0;
	}

	// Spawn blackhole
	if (Application::IsKeyPressed('G') && bounceTime > 1)
	{
		SpawnGO(GameObject::GO_BLACKHOLE, 1);
		bounceTime = 0;
	}

	// Mouse Section
	if (Application::IsMousePressed(0))	 // Firing
	{
		if (bTime_Weapon > m_ship->FireTime)
		{
			if (Weapon == MISSILE) SpawnGO(GameObject::GO_MISSILE, RoundsPerShot);	// Missile Spawning
			else				   SpawnGO(GameObject::GO_BULLET,  RoundsPerShot);	// Bullet Spawning

			bTime_Weapon = 0;
		}
	}

	// Repelling Force Field
	if (Application::IsKeyPressed(VK_SPACE))
	{
		if (RepelCooldown > 3.f && Repel_bTime == 0)
		{
			b_RepelActive = true;
			b_RepelActiveTime = true;

			RepelCooldown = 0;
		}
	}

	if (Application::IsKeyPressed('T'))
	{
		m_ship->HP -= DT * 4;
	}

	/*
	static bool bLButtonState = false;
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		//std::cout << "LBUTTON DOWN" << std::endl;
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		//std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		SpawnGO(GameObject::GO_POWERUP, 1);
		//std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		//std::cout << "RBUTTON UP" << std::endl;
	}
	*/

	//=================================================================================================
	//							  PHYSICS SIMULATION SECTION
	//=================================================================================================
	// SHIP UPDATE
	/*	 Ship Movement
			F = ma | a = F/m | a = F * (1/m)
	*/
	Vector3 acceleration = (m_force * (1 / m_ship->mass));
	m_ship->vel += acceleration * DT * m_speed;
	m_ship->pos += m_ship->vel * DT * m_speed;
	m_force.SetZero();

	if (m_ship->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)   // Clamp Ship Velocity
		m_ship->vel = m_ship->vel.Normalized() * MAX_SPEED;

	// Ship Rotation
	/*    Torque Formula
			 alpha = torque/I
			 dw = alpha * dt
			 dTheta = w * dt
	*/
	float alpha = m_torque.z / m_ship->momentOfInertia;
	m_ship->angularVelocity += alpha * m_speed * DT;

	m_ship->angularVelocity = Math::Clamp(m_ship->angularVelocity,	  // Clamp angular velocity
							(-MAX_ROTATE_SPEED), (MAX_ROTATE_SPEED)); // Min Max

	float theta = m_ship->angularVelocity * m_speed * DT * 8;

	m_ship->dir.Set((m_ship->dir.x * cos(theta)) - m_ship->dir.y * sin(theta),
					(m_ship->dir.x * sin(theta)) + m_ship->dir.y * cos(theta), 0);

	m_ship->Rotate = Math::RadianToDegree(atan2(m_ship->dir.y, m_ship->dir.x));

	// Ship Constrain
	//if (m_ship->pos.x < 3)				   { m_ship->pos.x = 3; }
	//if (m_ship->pos.x > m_worldWidth - 3)  { m_ship->pos.x = m_worldWidth - 3; }
	//if (m_ship->pos.y < 3)				   { m_ship->pos.y = 3; }
	//if (m_ship->pos.y > m_worldHeight - 3) { m_ship->pos.y = m_worldHeight - 3; }

	 WrapGO(m_ship);	// Ship Wrap Around
	//=================================================================================================
	for (auto &go : m_goList)
	{
		if (go->active)
		{
			//=========================================================================================
			//									   GAME LOGIC 
			//=========================================================================================
			if (WaveTime < 0)
			{
				RestartGame();
			}
			//=========================================================================================
			//								     BUDDY SHIP UPDATE	
			//=========================================================================================
			if (go->type == GameObject::GO_BUDDY)
			{
				// Movement
				if (bTime_Buddy < 0)
				{
					if (DistBetweenSquared(m_ship, go) < (22 * 22))
						go->pos += go->vel * dt * m_speed * BUDDY_SPEED;
					else
					{
						try {
							go->vel = (m_ship->pos - go->pos).Normalize();
							go->pos += go->vel * dt * m_speed * BUDDY_SPEED;
						}
						catch (DivideByZero) {}
					}

					bTime_Buddy = 1.5f;
				}
				else 
					go->pos += go->vel * dt * m_speed * BUDDY_SPEED;

				// Buddy ship Targeting
				if (go->Target != NULL)
				{
					if (go->Target->active)
					{
						if (DistBetween(go, go->Target) < 60) /* Within Firing Range*/
						{
							// Buddy ship Rotation
							try {
								Vector3 TempDir = (go->Target->pos - go->pos).Normalize();

								go->dir += (TempDir - go->dir) * 0.1f /* Buddy ship rotate speed*/;
								go->Rotate = Math::RadianToDegree(atan2(go->dir.y, go->dir.x));
							}
							catch (DivideByZero) {}

							// Buddy ship Firing
							if (go->FireTime < 0)
							{
								GameObject* go_Bullet = FetchGO();

								go_Bullet->scale.Set(0.2f, 0.2f, 0.2f);
								go_Bullet->pos = (go->pos + go->dir * 2);	// Spawn at front of ship
								go_Bullet->type = GameObject::GO_BULLET;
								go_Bullet->active = true;

								// Bullet Movement
								try {
									// go_Bullet->vel = (go->Target->pos - go_Bullet->pos).Normalize();
									go_Bullet->vel = go->dir.Normalize();
								}
								catch (DivideByZero) {}

								go_Bullet->vel.x += Math::RandFloatMinMax(-0.03f, 0.03f);
								go_Bullet->vel.y += Math::RandFloatMinMax(-0.03f, 0.03f);

								go->FireTime = BUDDY_FIRE_TIME;
							}
						}
					}
					else   // Retargeting
					{
						float TempDist = FLT_MAX;

						for (auto &go_Enemy : m_goList)
						{
							if (go_Enemy->active)
							{
								if (go_Enemy->type == GameObject::GO_ENEMY_SHIP)  // Find nearest enemy (within 50 units)
								{
									if (DistBetween(go_Enemy, m_ship) < TempDist)
									{
										TempDist = DistBetween(go_Enemy, m_ship);
										go->Target = go_Enemy;
									}
								}
							}
						}
					}
				}
				else   // Retargeting
				{
					float TempDist = FLT_MAX;

					for (auto& go_Enemy : m_goList)
					{
						if (go_Enemy->active)
						{
							if (go_Enemy->type == GameObject::GO_ENEMY_SHIP)  // Find nearest enemy (within 50 units)
							{
								if (DistBetween(go_Enemy, m_ship) < TempDist)
								{
									TempDist = DistBetween(go_Enemy, m_ship);
									go->Target = go_Enemy;
								}
							}
						}
					}
				}

				WrapGO(go);	// Wrap around
			}
			//=========================================================================================
			//								     ASTEROID UPDATE	
			//=========================================================================================
			if (go->type == GameObject::GO_ASTEROID)
			{
				// Collision with ship
				if ( isCollide(m_ship, go) )	
				{
					DamageGO(m_ship);	// Damage Ship
					DamageGO(go);		// Damage Asteroid

					continue;
				}

				go->pos += go->vel * DT * m_speed;	// Movement

				WrapGO(go);	// Wrap around
			}
			//=========================================================================================
			//									BULLET UPDATE	
			//=========================================================================================
			if (go->type == GameObject::GO_BULLET)
			{
				// Collision
				for (auto &go_Other : m_goList)
				{
					if (go_Other->active)
					{
						if (go_Other->type == GameObject::GO_ENEMY_SHIP ||	// Collide with Enemy Ship
							go_Other->type == GameObject::GO_ASTEROID)		// Collide with Asteroid
						{
							if (isCollide(go, go_Other))
							{
								if (PenPow <= 1) { go->active = false; }
								else { --PenPow; }

								DamageGO(go_Other);		// Damage Enemy

								break;
							}
						}
					}
				}

				if (outOfBounds(go)) { go->active = false; }	// Despawn when leave screen

				go->pos += go->vel * DT * m_speed * BULLET_SPEED;	// Movement
			}
			//=========================================================================================
			//									MISSILE UPDATE 
			//=========================================================================================
			if (go->type == GameObject::GO_MISSILE)
			{
				// Collision 
				for (auto &go_Other : m_goList)	
				{
					if (go_Other->active)
					{
						if (go_Other->type == GameObject::GO_ENEMY_SHIP ||	// Collision with Enemy Ship
							go_Other->type == GameObject::GO_ASTEROID)		// Collision with Asteroid
						{
							if (isCollide(go, go_Other))
							{
								go->active = false;			// Despawn Missile
								go_Other->active = false;	// Despawn GameObject

								break;
							}
						}
					}
				}

				// Missile Homing
				if (go->Target != NULL)
				{
					if (go->Target->active)
					{
						try {
							go->dir = (go->Target->pos - go->pos).Normalized();
						}
						catch (DivideByZero) {}
					}
					else   // Missile Retargeting
					{
						float TempDist = FLT_MAX;

						for (auto &go_Enemy : m_goList)
						{
							if (go_Enemy->active)
							{
								if (go_Enemy->type == GameObject::GO_ENEMY_SHIP)  // Find nearest enemy
								{
									if (DistBetween(go_Enemy, m_ship) < TempDist)	
									{
										TempDist = DistBetween(go_Enemy, m_ship);
										go->Target = go_Enemy;
									}
								}
							}
						}
					}
				}

				if (outOfBounds(go)) { go->active = false; }	// Despawn when leave screen

				go->Rotate = Math::RadianToDegree( atan2(go->dir.y, go->dir.x) );	// Missile Rotation

				go->pos += go->dir * DT * m_speed * MISSILE_SPEED;	// Movement
			}
			//=========================================================================================
			//									ENEMY UPDATE 
			//=========================================================================================
			if (go->type == GameObject::GO_ENEMY_SHIP)
			{
				// Collision with Ship
				if (isCollide(m_ship, go))
				{
					DamageGO(go);		// Damage Enemy
					DamageGO(m_ship);	// Damage Ship

					continue;
				}

				// Enemy Movement		
				try {
					// Cap Movement speed
					if ( (go->vel + (m_ship->pos - go->pos).Normalized()).Length() < MAX_ENEMY_SPEED )
					{
						go->vel += (m_ship->pos - go->pos).Normalize();
					}
				}
				catch (DivideByZero) {}

				go->pos += go->vel * m_speed * ENEMY_SPEED * DT;

				// Enemy Rotation
				Vector3 Target = (m_ship->pos - go->pos);
				go->Rotate = Math::RadianToDegree(atan2(Target.y, Target.x));

				// Enemy Firing
				if (DistBetweenSquared(m_ship, go) < (50 * 50))		// If within 50 units 
				{
					if (go->FireTime <= 0.f)
					{
						GameObject* eBullet = FetchGO();
				
						try {
							eBullet->vel = (m_ship->pos - go->pos).Normalize();
						}
						catch (DivideByZero) {}
				
						eBullet->type = GameObject::GO_ENEMY_BULLET;
						eBullet->pos = go->pos + (eBullet->vel * 3);
						eBullet->scale.Set(0.85f, 0.85f, 0.85f);
						eBullet->active = true;
	
						go->FireTime = ENEMY_FIRE_TIME;
					}
				}

				WrapGO(go);	// Wrap around
			}
			//=========================================================================================
			//								  ENEMY BULLET UPDATE 
			//=========================================================================================
			if (go->type == GameObject::GO_ENEMY_BULLET)
			{
				// Collision
				if (isCollide(go, m_ship))
				{
					go->active = false;		// Unspawn enemy bullet
					DamageGO(m_ship);		// Damage ship

					continue;
				}

				go->pos += go->vel * DT * m_speed * ENEMY_BULLET_SPEED;	 // Movement		

				if (outOfBounds(go)) { go->active = false; }	// Despawn bullet when out of screen
			}
			//=========================================================================================
			//						      		POWERUP UPDATE
			//=========================================================================================
			if (go->type == GameObject::GO_POWERUP)
			{
				if (isCollide(m_ship, go))	// Collision with m_Ship
				{
					go->active = false;		// Despawn power-up
					m_ship->HP = SHIP_HP;	// Refill HP
				}
			}
			//=========================================================================================
			//						      		BLACKHOLE UPDATE
			//=========================================================================================
			if (go->type == GameObject::GO_BLACKHOLE)
			{
				/*	Newton's Universal Law of Gravitation
						F1 = F2 = G[(m1 + m2)/d^2]
				*/
				float GravitationalConstant = 35;

				for (auto &go_Other : m_goList)
				{
					if (go_Other->active)
					{
						if (go_Other->type != GameObject::GO_BLACKHOLE)
						{
							// Exerting gravitational pull on GOs in m_goList
							float F = (GravitationalConstant * (go->mass + go_Other->mass))
									  /(go->pos - go_Other->pos).LengthSquared();

							try {
								Vector3 Dir = (go->pos - go_Other->pos).Normalize();
								go_Other->vel += Dir * F;
							}
							catch (DivideByZero) {}

							// Collision with GOs in m_goList
							if (DistBetweenSquared(go, go_Other) < (5 * 5))
							{
								go_Other->active = false;
							}
						}
					}
				}

				// Exerting gravitational pull on m_ship
				float F = (GravitationalConstant * (go->mass + m_ship->mass))
				 	  	  /(m_ship->pos - go->pos).LengthSquared();

				try {
					Vector3 Dir = (go->pos - m_ship->pos).Normalize();
					m_ship->vel += Dir * F * 10;
				}
				catch (DivideByZero) {}

				// Collision with m_ship
				if (DistBetweenSquared(go, m_ship) < (5 * 5))
					RestartGame();	

				// If timer runs out, Blackhole despawns
				if (go->FireTime <= 0)
					go->active = false;

				go->pos += go->vel * DT * m_speed;	// Movement

				WrapGO(go);	// Blackhole Wrap around
			}
			//=========================================================================================
			//						       BULLET REPULSION FORCE FIELD
			//=========================================================================================
			if (b_RepelActive)
			{
				for (auto &GO : m_goList)
				{
					if (GO->active)
					{
						if (GO->type == GameObject::GO_ENEMY_BULLET)
						{
							if (RepelTime > 0)
							{
								// Force	 Inversely Proportional To	   Distance
								int Pow = 40;
								float F = (Pow / DistBetween(m_ship, GO));

								try {
									Vector3 Dir = (GO->pos - m_ship->pos).Normalize();
									GO->vel.SetZero();
									GO->vel += Dir * F;
								}
								catch (DivideByZero) {}
							}
						}
					}
				}
				
				b_RepelActive = false;
			}

			if (b_RepelActiveTime)
			{
				if (Repel_bTime > 3.f)
				{
					b_RepelActiveTime = false;
					Repel_bTime = 0;
				}
			}
			//=========================================================================================
		}

		go->FireTime -= DT;
	}

	// Keep count of everything
	NumActive = 0, NumEnemy = 0, NumAsteroid = 0;

	for (auto &go : m_goList)	
	{
		if (go->active)	
		{																
			++NumActive;												//	Number of active GameObjects
																		
			if (go->type == GameObject::GO_ENEMY_SHIP)	++NumEnemy;		//	Number of Enemies
			if (go->type == GameObject::GO_ASTEROID)	++NumAsteroid;	//	Number of Asteroids
		}
	}

	//// Misc Funcs
	// Random Asteroid spawn
	if (AsteroidRandTime < 0)
	{
		SpawnGO(GameObject::GO_ASTEROID, CurrWave * 2);
		AsteroidRandTime = Math::RandFloatMinMax(3, 8);
	}

	// Random Enemy spawn
	if (EnemyRandTime < 0)
	{
		SpawnGO(GameObject::GO_ENEMY_SHIP, CurrWave * 2);
		EnemyRandTime = Math::RandFloatMinMax(3, 8);
	}

	// Random Powerup spawn
	if (pwrUpRandTime < 0)
	{
		if (Math::RandIntMinMax(1, 5) == 3)	
			SpawnGO(GameObject::GO_POWERUP, 1);

		pwrUpRandTime = 10;
	}

	// Random Blackhole Spawn
	if (BlackHole_RandTime < 0)
	{
		if (Math::RandIntMinMax(1, 6) == 3)
			SpawnGO(GameObject::GO_BLACKHOLE, 1);

		BlackHole_RandTime = Math::RandFloatMinMax(5, 25);
	}

	// Custom stuffs
	m_ship->FireTime -= DT;
	bTime_Weapon += DT;
	bTime_Switch += DT;
	AsteroidRandTime -= DT;
	EnemyRandTime -= DT;
	pwrUpRandTime -= DT;
	BlackHole_RandTime -= DT;
	RepelCooldown += DT;
	bTime_Buddy -= DT;
	WaveTime -= DT;

	if (b_RepelActiveTime)
		Repel_bTime += DT;

	// DEBUG	// TODO : Delete
	// std::cout << bTime_Weapon << std::endl;
	// std::cout << ship_Rotate << std::endl;	
	// std::cout << m_ship->angularVelocity << std::endl;
	// std::cout << "Vel: " << m_ship->vel * (0.1) << std::endl;
	// std::cout << AsteroidRandTime << std::endl;
	// std::cout << RepelTime << std::endl;
	// std::cout << bTime_Buddy << std::endl;
	// std::cout << m_ship->FireTime << std::endl;
	// std::cout << Repel_bTime << std::endl;
	// std::cout << m_ship->mass << std::endl;
	// std::cout << "x: " << m_ship->pos.x - (m_worldWidth	/ 2) << std::endl;
	// std::cout << "y: " << m_ship->pos.y - (m_worldHeight / 2) << std::endl;
}

void SceneAsteroid::RenderGO(GameObject *go, float Z)
{
	switch(go->type)
	{
		case GameObject::GO_SHIP:	  // Ship
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Rotate(m_ship->Rotate, 0, 0, 1);
				modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
				RenderMesh(meshList[GEO_SHIP], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_BUDDY:	  // Buddy Ship
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Rotate(go->Rotate, 0, 0, 1);
				modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
				RenderMesh(meshList[GEO_BUDDY_SHIP], false);	
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_ASTEROID:  // Asteroid
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Scale(go->scale.x * 4, go->scale.y * 4, go->scale.z);
				RenderMesh(meshList[GEO_ASTEROID], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_BULLET:		// Ship Bullet
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Scale(go->scale.x * 1.3, go->scale.y * 1.3, go->scale.z);
				RenderMesh(meshList[GEO_BALL], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_MISSILE:	 // Ship Homing Missile
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Rotate(go->Rotate, 0, 0, 1);
				modelStack.Scale(go->scale.x * 2.f, go->scale.y * 2.f, go->scale.z);
				RenderMesh(meshList[GEO_MISSILE], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_ENEMY_SHIP:	  // Enemy Ship	 
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Rotate(go->Rotate, 0, 0, 1);
				modelStack.Scale(go->scale.x * 2, go->scale.y * 2, go->scale.z);
				RenderMesh(meshList[GEO_ENEMY_SCOUT], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_ENEMY_BULLET:	// Enemy Bullet
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Scale(go->scale.x * 0.3f, go->scale.y * 0.3f, go->scale.z * 1);
				RenderMesh(meshList[GEO_RED_BALL], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_POWERUP:	// Powerup
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
				RenderMesh(meshList[GEO_POWERUP], false);
			modelStack.PopMatrix();
			break;
		}
		case GameObject::GO_BLACKHOLE:	  // Black hole
		{
			modelStack.PushMatrix();
				modelStack.Translate(go->pos.x, go->pos.y, Z);
				modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
				RenderMesh(meshList[GEO_BLACKHOLE], false);
			modelStack.PopMatrix();
			break;
		}
		default:  std::cout << "RenderGO IS SCREWED " << std::endl;  break;
	}
}

void SceneAsteroid::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glDisable(GL_DEPTH_TEST);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);
	
	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
						camera.position.x, camera.position.y, camera.position.z,
						camera.target.x, camera.target.y, camera.target.z,
						camera.up.x, camera.up.y, camera.up.z
					);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	// Render background
	// Parrallax Scrolling
	float xOffset = m_ship->pos.x - (m_worldWidth / 2);
	float yOffset = m_ship->pos.y - (m_worldHeight / 2);

	float xTranslate = (m_worldWidth / 2) - xOffset;
	float yTranslate = (m_worldHeight / 2) - yOffset;

	Vector3 V(xTranslate, yTranslate);

	if (xTranslate > (m_worldWidth / 2))
		V.x -= m_worldWidth;
	if (xTranslate < (m_worldWidth / 2))
		V.x += m_worldWidth;
	if (yTranslate > (m_worldHeight / 2))
		V.y -= m_worldHeight;
	if (yTranslate < (m_worldHeight / 2))
		V.y += m_worldHeight;

	// Center
	modelStack.PushMatrix();
		modelStack.Translate(xTranslate, yTranslate, 0);
		modelStack.Scale(m_worldWidth * 1, m_worldHeight * 1, 1);	
		RenderMesh(meshList[GEO_BACKGROUND], false);
	modelStack.PopMatrix();

	// Left Right
	modelStack.PushMatrix();
		modelStack.Translate(V.x, yTranslate, 0);
		modelStack.Scale(m_worldWidth * 1, m_worldHeight * 1, 1);	
		RenderMesh(meshList[GEO_BACKGROUND], false);
	modelStack.PopMatrix();

	// Top - Bottom
	modelStack.PushMatrix();
		modelStack.Translate(xTranslate, V.y, 0);
		modelStack.Scale(m_worldWidth * 1, m_worldHeight * 1, 1);	
		RenderMesh(meshList[GEO_BACKGROUND], false);
	modelStack.PopMatrix();

	// Corners
	modelStack.PushMatrix();
		modelStack.Translate(V.x, V.y, 0);
		modelStack.Scale(m_worldWidth * 1, m_worldHeight * 1, 1);	
		RenderMesh(meshList[GEO_BACKGROUND], false);
	modelStack.PopMatrix();

	// Bullet Repulsion Force-Field Effect
	modelStack.PushMatrix();
		modelStack.Translate(m_ship->pos.x, m_ship->pos.y, Math::EPSILON);
		modelStack.Scale(Repel_bTime * 85, Repel_bTime * 85, 0);
		RenderMesh(meshList[GEO_FORCE_FIELD], false);
	modelStack.PopMatrix();

	Z = Math::EPSILON;

	for (auto &go : m_goList)
	{
		Z += Math::EPSILON;

		if(go->active)
			RenderGO(go, Z);
	}

	Z += Math::EPSILON;

	if (m_ship != NULL)		// Render Ship
		RenderGO(m_ship, Z);

	//On screen text
	std::ostringstream ss;
	ss.precision(3);
	ss << "Time Left: " << WaveTime;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 4);

	ss.str("");
	ss.precision(3);
	ss << "HP: " << m_ship->HP;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 40, 4);

	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 58);

	ss.str("");
	ss.precision(3);
	ss << "m_lives: " << m_lives; 
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 7);

	ss.str("");
	ss.precision(3);
	ss << "m_score: " << m_score;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 10);

	ss.str("");
	ss.precision(3);
	ss << "Wave " << CurrWave;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 13);

	ss.str("");
	ss.precision(3);
	ss << "Enemies:" << NumEnemy;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 16);

	//ss.str("");
	//ss.precision(3);
	//ss << "Weapon: " << CurrWeapon;
	//RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 19);

	ss.str("");
	ss.precision(3);
	ss << "GO Count: " << NumActive;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 2.2, 0, 19);
	
	// RenderTextOnScreen(meshList[GEO_TEXT], "Asteroid", Color(1, 1, 0), 2.2, 0, 0);
	ss.clear();

	// Crosshair : Put last so it will render over everything else
	modelStack.PushMatrix();
		modelStack.Translate(CurrMouseX, CurrMouseY, 1.f);
		modelStack.Scale(11, 11, 1);
		RenderMesh(meshList[GEO_CROSSHAIR], false);
	modelStack.PopMatrix();
}

void SceneAsteroid::Exit()
{
	SceneBase::Exit();

	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ship)
	{
		delete m_ship;
		m_ship = NULL;
	}
}

// Misc Functions
bool SceneAsteroid::isCollide(GameObject* go, GameObject* go2)
{
	return (go->pos - go2->pos).LengthSquared() < 
		(go->scale.x + go2->scale.x) * (go->scale.x + go2->scale.x);
}

bool SceneAsteroid::outOfBounds(GameObject* go)
{
	return (go->pos.x > m_worldWidth || 
			go->pos.x < 0 || 
			go->pos.y > m_worldHeight || 
			go->pos.y < 0);
}

void SceneAsteroid::WrapGO(GameObject* go)
{
	if (go->pos.x > m_worldWidth)	{ go->pos.x -= m_worldWidth;  }
	if (go->pos.x < 0)				{ go->pos.x += m_worldWidth;  }
	if (go->pos.y > m_worldHeight)	{ go->pos.y -= m_worldHeight; }
	if (go->pos.y < 0)				{ go->pos.y += m_worldHeight; }
}

float SceneAsteroid::DistBetween(GameObject* go, GameObject* go2)
{
	return ( (go->pos - go2->pos).Length() );
}

float SceneAsteroid::DistBetweenSquared(GameObject* go, GameObject* go2)
{
	return ((go->pos - go2->pos).LengthSquared());
}

int SceneAsteroid::GetQuadrant(GameObject* go)
{
	float xPos = go->pos.x - (m_worldWidth / 2);	// X Offset
	float yPos = go->pos.y - (m_worldHeight / 2);	// Y Offset

	if		(xPos == 0 && yPos == 0) { return 0; }	// World Centre/Origin
	else if	(xPos >= 0 && yPos >= 0) { return 1; }  // Quadrant 1, Top Right
	else if (xPos <= 0 && yPos >= 0) { return 2; }  // Quadrant 2, Top Left
	else if (xPos <= 0 && yPos <= 0) { return 3; }  // Quadrant 3, Bottom Left
	else if (xPos >= 0 && yPos <= 0) { return 4; }  // Quadrant 4, Bottom Right
	else							 { return -1; } // Bad
}

void SceneAsteroid::SetWeapon(WEAPON_TYPE w)
{
	// Edit gun properties here
	switch (w)
	{
		case RIFLE:		// Fast firing, medium accuracy
		{
			RoundsPerShot = 1;
			PenPow = 1;
			m_ship->FireTime = 0.03f;
			Spread = 0.05f;
			CurrWeapon = "Rifle";

			break;
		}
		case SHOTGUN:	// Large spread, low range, low firerate
		{
			RoundsPerShot = 25;
			PenPow = 1;
			m_ship->FireTime = 0.85f;
			Spread = 0.18f;
			CurrWeapon = "Shotgun";

			break;
		}
		case SNIPER:	// High penetration/accuracy, low firerate
		{
			RoundsPerShot = 1;
			PenPow = 3;
			m_ship->FireTime = 0.7f;
			Spread = 0;
			CurrWeapon = "Sniper";

			break;
		}
		case MISSILE:	// Homing
		{
			RoundsPerShot = 1;
			PenPow = 1;
			m_ship->FireTime = 2.f;
			Spread = 0;
			CurrWeapon = "Missile";

			break;
		}
		default: std::cout << "GUN CODE IS BUGGY " << std::endl; break;
	}
}

void SceneAsteroid::DamageGO(GameObject* go)
{
	if (go->HP > 1) { --go->HP; }
	else
	{
		if (go->type == GameObject::GO_SHIP)
			RestartGame();
		else
		{
			go->active = false;
			m_score += 2;
		}
	}
}

void SceneAsteroid::SpawnGO(GameObject::GAMEOBJECT_TYPE goType, int Num)
{
	switch (goType)
	{
		case GameObject::GO_BULLET:
		{
			for (int i = 0; i < Num; ++i)
			{
				GameObject* go_Bullet = FetchGO();

				go_Bullet->scale.Set(0.18f, 0.18f, 0.18f);
				go_Bullet->pos = m_ship->pos + (m_ship->dir * 3);	// Spawn at front of ship
				go_Bullet->vel.x = CurrMouseX - go_Bullet->pos.x;
				go_Bullet->vel.y = CurrMouseY - go_Bullet->pos.y;
				go_Bullet->type = goType;
				go_Bullet->mass = 0.3f;
				go_Bullet->active = true;

				try {
					go_Bullet->vel.Normalize();
				}
				catch (DivideByZero) {}

				go_Bullet->vel.x += Math::RandFloatMinMax(-Spread, Spread);
				go_Bullet->vel.y += Math::RandFloatMinMax(-Spread, Spread);
			}

			break;
		}
		case GameObject::GO_BUDDY:
		{
			bool b_Buddy = false;

			for (auto& go : m_goList)
			{
				if (go->active)
				{
					if (go->type == GameObject::GO_BUDDY)
					{
						b_Buddy = true;
					}
				}
			}

			if (b_Buddy == false)
			{
				GameObject* go_Buddy = FetchGO();

				go_Buddy->type = goType;
				go_Buddy->scale.Set(6.f, 5.5f, 1.f);
				go_Buddy->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), Math::RandFloatMinMax(0, m_worldHeight), 0);
				go_Buddy->vel.SetZero();
				go_Buddy->FireTime = 1.0f;
				go_Buddy->Target = NULL;
				go_Buddy->mass = 1;
				go_Buddy->active = true;

				// Buddy ship (initial) targeting
				if (go_Buddy->Target == NULL)
				{
					float TempDist = 40;

					for (auto& go_Enemy : m_goList)
					{
						if (go_Enemy->active)
						{
							if (go_Enemy->type == GameObject::GO_ENEMY_SHIP)
							{
								if (DistBetween(go_Buddy, go_Enemy) < TempDist)	// Find nearest enemy
								{
									TempDist = DistBetween(go_Buddy, go_Enemy);
									go_Buddy->Target = go_Enemy;
								}
							}
						}
					}
				}
			}

			break;
		}
		case GameObject::GO_ASTEROID: 
		{
			if ( (NumAsteroid + Num) < MAX_ASTEROID)
			{
				for (int i = 0; i < Num; ++i)
				{
					GameObject* GO = FetchGO();

					float RandAsteroid = Math::RandFloatMinMax(0.8f, 3.5f);

					int Wall = Math::RandIntMinMax(1, 4);

					switch (Wall)
					{
						case 1:		// Left 
							GO->pos.Set(0, Math::RandFloatMinMax(0, m_worldHeight), 0);
							GO->vel.Set(Math::RandFloatMinMax(0.5f, 5.f), Math::RandFloatMinMax(-5.f, 5.f), 0);
							break;
						case 2:		// Top
							GO->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), m_worldHeight, 0);
							GO->vel.Set(Math::RandFloatMinMax(-5.f, 5.f), Math::RandFloatMinMax(-5.f, -0.5f), 0);
							break;
						case 3:		// Bottom
							GO->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), 0, 0);
							GO->vel.Set(Math::RandFloatMinMax(-5.f, 5.f), Math::RandFloatMinMax(0.5f, 5.f), 0);
							break;
						case 4:		// Right
							GO->pos.Set(m_worldWidth, Math::RandFloatMinMax(0, m_worldHeight), 0);
							GO->vel.Set(Math::RandFloatMinMax(-5.f, -0.5f), Math::RandFloatMinMax(-5.f, 5.f), 0);
							break;
						default: std::cout << "ASTEROID SPAWNING BROKEN " << std::endl; break;
					}

					GO->HP = ASTEROID_HP;
					GO->type = goType;
					GO->scale.Set(RandAsteroid, RandAsteroid, 1.f);
					GO->mass = RandAsteroid / 1.5f;
					GO->active = true;
				}
			}

			break;
		}
		case GameObject::GO_ENEMY_SHIP:		
		{
			if ( (NumEnemy + Num) < MAX_ENEMY)
			{
				for (int i = 0; i < Num; ++i)
				{
					GameObject* goEnemy = FetchGO();

					int Wall = Math::RandIntMinMax(1, 4);

					switch (Wall)
					{
						case 1:		// Left 
							goEnemy->pos.Set(30, Math::RandFloatMinMax(0, m_worldHeight), 0);
							break;
						case 2:		// Top
							goEnemy->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), m_worldHeight, 0);
							break;
						case 3:		// Bottom
							goEnemy->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), 0, 0);
							break;
						case 4:		// Right
							goEnemy->pos.Set(m_worldWidth, Math::RandFloatMinMax(0, m_worldHeight), 0);
							break;
						default: std::cout << "ENEMY SPAWN IS POSSIBLY BROKEN " << std::endl; break;
					}

					goEnemy->FireTime = ENEMY_FIRE_TIME;
					goEnemy->HP = ENEMY_HP;
					goEnemy->type = goType;
					goEnemy->scale.Set(2.2f, 2.2f, 2.2f);
					goEnemy->mass = 5;
					goEnemy->active = true;
				}
			}

			break;
		}
		case GameObject::GO_MISSILE:	
		{
			for (int i = 0; i < Num; ++i)
			{
				GameObject* goMissile = FetchGO();

				goMissile->type = goType;
				goMissile->pos = m_ship->pos + (m_ship->dir * 3);	// Spawn at front of ship
				goMissile->scale.Set(1.8f, 1.0f, 2.4f);
				goMissile->Target = NULL;
				goMissile->mass = 1;
				goMissile->active = true;

				try {
					goMissile->dir = goMissile->vel.Normalized();
				}
				catch (DivideByZero) {}

				// Missile Targeting
				if (goMissile->Target == NULL)
				{
					float TempDist = FLT_MAX;

					for (auto &go_Enemy : m_goList)
					{
						if (go_Enemy->active)
						{
							if (go_Enemy->type == GameObject::GO_ENEMY_SHIP)
							{
								if (DistBetween(go_Enemy, m_ship) < TempDist)	// Find nearest enemy
								{
									TempDist = DistBetween(go_Enemy, m_ship);
									goMissile->Target = go_Enemy;
								}
							}
						}
					}
				}
			}

			break;
		}
		case GameObject::GO_BLACKHOLE:	
		{
			// Check if a blackhole already exists
			bool b_BlackHole = false;

			for (auto &go : m_goList)
			{
				if (go->active)
				{
					if (go->type == GameObject::GO_BLACKHOLE)
					{
						b_BlackHole = true;
					}
				}
			}

			// else, make one
			if (b_BlackHole == false)
			{
				for (int i = 0; i < Num; ++i)
				{
					GameObject* go_BH = FetchGO();

					go_BH->type = GameObject::GO_BLACKHOLE;
					go_BH->scale.Set(30.f, 18.f, 4.f);
					go_BH->mass = 5;
					go_BH->FireTime = Math::RandFloatMinMax(8, 15);	// Range of seconds black hole will last
					go_BH->active = true;
				
					go_BH->vel.Set(Math::RandFloatMinMax(-3.f, 3.f),
							       Math::RandFloatMinMax(-3.f, 3.f), 0);

					int xOffset = 20;
					int yOffset = 12;

					/*
						If Ship at center of world, spawn in random corner
						Else, spawn in quadrant opposite ship
					*/
					switch (GetQuadrant(m_ship))
					{
						case 0: 
						{		
							int Corner = Math::RandIntMinMax(1, 4);

							switch (Corner)
							{
								case 1:	    // Top left
									go_BH->pos.Set(xOffset, m_worldHeight - yOffset, 0);
									break;
								case 2:		// Top Right
									go_BH->pos.Set(m_worldWidth - xOffset, m_worldHeight - yOffset, 0);
									break;
								case 3:		// Buttom Left
									go_BH->pos.Set(xOffset, yOffset, 0);
									break;
								case 4:		// Bottom right
									go_BH->pos.Set(m_worldWidth - xOffset, yOffset, 0);
									break;
								default: std::cout << "BLACKHOLE SPAWNING IS UNOPTIMAL.....and BROKEN " << std::endl;
							}
						}
						case 1:	// Quadrant 1, Top Right
							go_BH->pos.Set(xOffset, yOffset, 0);	// Spawn bottom left
							break;
						case 2:	// Quadrant 2, Top Left
							go_BH->pos.Set(m_worldWidth - xOffset, yOffset, 0);	// Spawn Bottom Right
							break;
						case 3:	// Quadrant 3, Bottom Left
							go_BH->pos.Set(m_worldWidth - xOffset, m_worldHeight - yOffset, 0);	// Spawn Top Right
							break;
						case 4:	// Quadrant 4, Bottom Right
							go_BH->pos.Set(xOffset, m_worldHeight - yOffset, 0); // Spawn top left
							break;
						case -1:  std::cout << "GetQuadrant FUNCTION IS BAD " << std::endl;  break;
						default:  std::cout << "BLACKHOLE SPAWNING TO SHREDS " << std::endl; break;
					}
				}
			}

			break;
		}
		case GameObject::GO_POWERUP:	
		{
			for (int i = 0; i < Num; ++i)
			{
				GameObject* go_pwrUP = FetchGO();

				int Offset = 30;

				go_pwrUP->pos.Set(Math::RandFloatMinMax(Offset, m_worldWidth - Offset),
							   	  Math::RandFloatMinMax(Offset, m_worldHeight - Offset), 0);

				go_pwrUP->type = goType;
				go_pwrUP->scale.Set(4.f, 4.f, 4.f);
				go_pwrUP->active = true;

				break;
			}
		}
		default: std::cout << "POWERUP SPAWNING IS BAD " << std::endl;	break;	
	}
}

void SceneAsteroid::RestartGame()
{
	// End Game condition
	if (m_lives == 0)
	{
		if (WaveTime >= 0)
		{
			// Died
			Application::Score = m_score;
			Application::Wave = CurrWave;

			Application::GameEnd = true;
			/*std::cout << "DIED " << std::endl;
			std::cout << "Final Score: " << m_score << std::endl;
			std::cout << "Wave: " << CurrWave << std::endl;*/
		}
		else 
			++CurrWave;
	}
	else
	{
		if (WaveTime >= 0) 
			--m_lives;
		else 
			++CurrWave;
	}

	// Set entirety of m_goList to inactive
	for (auto& matic : m_goList)
		if (matic->active) { matic->active = false; }

	m_torque.SetZero();		
	m_force.SetZero();		

	m_ship->HP = SHIP_HP;
	m_ship->pos.Set(m_worldWidth/2, m_worldHeight/2, 0);
	m_ship->vel.SetZero();
	m_ship->dir.Set(1, 0, 0);
	m_ship->momentOfInertia = (m_ship->mass * m_ship->scale.x * m_ship->scale.x);	
	m_ship->angularVelocity = 0;

	AsteroidRandTime = 0, EnemyRandTime = 0, pwrUpRandTime = 15;

	NumActive = 0, NumEnemy = 0, NumAsteroid = 0;

	DT = 0;	
	RepelTime = 3.f;
	Repel_bTime = 0;
	RepelCooldown = 0;
	b_RepelActive = false;
	b_RepelActiveTime = false;
	bTime_Buddy = 1.5f;
	BlackHole_RandTime = Math::RandFloatMinMax(10, 20);

	WaveTime = ROUND_TIME;

	GameEnd = false;

	SpawnGO(GameObject::GO_ASTEROID, CurrWave * 2);	// Asteroid   (re)Init
	SpawnGO(GameObject::GO_ENEMY_SHIP, CurrWave * 2);	// Enemy	  (re)Init
	SpawnGO(GameObject::GO_BUDDY, 1);		// Buddy Ship (re)Init
}