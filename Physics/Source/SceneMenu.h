#ifndef SCENE_MENU_H
#define SCENE_MENU_H

#include "SceneBase.h"

class SceneMenu : public SceneBase
{
private:
	float m_worldWidth;
	float m_worldHeight;

public:
	SceneMenu();
	~SceneMenu();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();
};

#endif