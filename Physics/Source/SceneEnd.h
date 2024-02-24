#ifndef SCENE_END_H
#define SCENE_END_H

#include "SceneBase.h"

class SceneEnd : public SceneBase
{
private:
	float m_worldWidth;
	float m_worldHeight;

public:
	SceneEnd();
	~SceneEnd();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();
};

#endif