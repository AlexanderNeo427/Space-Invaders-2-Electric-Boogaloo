
#include "GameObject.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue)
	: type(typeValue),
	scale(1, 1, 1),
	active(false),
	mass(1.f),
	dir(1, 0, 0),
	momentOfInertia(1),
	angularVelocity(0),
	FireTime(0),
	Rotate(0)
{
}

GameObject::~GameObject()
{
}