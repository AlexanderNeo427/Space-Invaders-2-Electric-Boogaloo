#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Vector3.h"

struct GameObject
{
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,
		GO_BALL,
		GO_CUBE,

		GO_ASTEROID,    
		GO_SHIP,	    
		GO_BULLET,		
		GO_ENEMY, 
		GO_ENEMY_BULLET,
		GO_MISSILE,
		GO_POWERUP,
		GO_ENEMY_SHIP,
		GO_BUDDY,
		GO_BLACKHOLE,

		GO_TOTAL, 
	};

	GAMEOBJECT_TYPE type;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;

	bool active;
	float mass;

	Vector3 dir;			// Direction / Orientation
	float momentOfInertia;
	float angularVelocity;	// In Radians

	// Custom Stuffs
	int HP;
	double FireTime;
	GameObject* Target;
	float Rotate;

	GameObject(GAMEOBJECT_TYPE typeValue = GO_BALL);
	~GameObject();
};

#endif