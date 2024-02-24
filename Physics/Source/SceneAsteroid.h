#ifndef SCENE_ASTEROID_H
#define SCENE_ASTEROID_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneAsteroid : public SceneBase
{
	//-------------- Ship --------------------
	const float MAX_ROTATE_SPEED = 2.f;
	static const int SHIP_HP = 100;
	static const int MAX_SPEED = 15;
	static const int BULLET_SPEED = 80;
	static const int MISSILE_SPEED = 25;
	static const int MISSILE_POWER = 1;
	static const int NUM_LIVES = 3;
	//--------------- Enemy ------------------
	const float ENEMY_SPEED = 0.4f;	
	const float MAX_ENEMY_SPEED = 25.0f;
	const float ENEMY_FIRE_TIME = 0.7f;
	static const int ENEMY_BULLET_SPEED = 18;
	static const int MAX_ENEMY = 1000;
	static const int ENEMY_HP = 8;
	static const int MAX_ASTEROID = 80;
	//-------------- Asteroid -----------------
	static const int ASTEROID_HP = 8;
	//--------------- Buddy Ship --------------
	const float BUDDY_FIRE_TIME = 0.2f;
	const static int BUDDY_SPEED = 12;
	const static int MAX_BUDDY_SHIP = 3;
	//---------------- Misc -------------------
	const static int ROUND_TIME = 30;
	// const static int MAX_WAVES = 3;

public:
	SceneAsteroid();
	~SceneAsteroid();

	enum WEAPON_TYPE
	{
		RIFLE = 0,
		SHOTGUN,
		SNIPER,
		MISSILE,

		NUM_WEAPON_TYPES
	} Weapon;

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go, float Z);
	GameObject* FetchGO();

	// Misc Funcs
	bool isCollide(GameObject* go, GameObject* go2);
	bool outOfBounds(GameObject* go);
	float DistBetween(GameObject* go, GameObject* go2);
	float DistBetweenSquared(GameObject* go, GameObject* go2);
	int GetQuadrant(GameObject* go);
	void SetWeapon(WEAPON_TYPE w);
	void WrapGO(GameObject* go);
	void DamageGO(GameObject* go);
	void SpawnGO(GameObject::GAMEOBJECT_TYPE, int Num);
	void RestartGame();

protected:
	// Physics
	std::vector<GameObject*>m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ship;
	Vector3 m_force;
	int m_objectCount;
	int m_lives;
	int m_score;

	// Misc
	Vector3 m_torque;
private:	
	int RoundsPerShot, PenPow, NumMissile;
	float Spread;		
	float bTime_Weapon, bTime_Switch;
	std::string CurrWeapon;

	bool b_RepelActive, b_RepelActiveTime;
	float RepelTime, Repel_bTime, RepelCooldown;

	float bTime_Buddy;
	float BlackHole_RandTime;
	float WaveTime;

	// Mouse Coords
	double x, y;
	float CurrMouseX, CurrMouseY;

	// Misc 
	bool GameEnd;
	int NumActive, NumEnemy, NumAsteroid;
	float DT;
	float Z;
	int CurrWave;

	float AsteroidRandTime, EnemyRandTime, pwrUpRandTime;
};

#endif