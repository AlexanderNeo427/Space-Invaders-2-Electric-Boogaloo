#include "SceneEnd.h"

#include "GL\glew.h"
#include "Application.h"

#include <sstream>
#include <conio.h>

SceneEnd::SceneEnd()
{
}

SceneEnd::~SceneEnd()
{
}

void SceneEnd::Init()
{
	SceneBase::Init();

	// Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
}

void SceneEnd::Update(double dt)
{
	SceneBase::Update(dt);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	// RenderMesh(meshList[GEO_AXES], false);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldWidth / 2, m_worldHeight / 2, 0);
	modelStack.Scale(m_worldWidth * 1.3, m_worldHeight, 1);
	RenderMesh(meshList[GEO_MENU], false);
	modelStack.PopMatrix();

	//On screen text
	std::ostringstream ss;
	ss.precision(3);
	ss << "GAME END";
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 10, 10, 48);

	ss.str("");
	ss.precision(3);
	ss << "Wave: " << Application::Wave;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 4.3f, 15, 20);

	ss.str("");
	ss.precision(3);
	ss << "Score: " << Application::Score;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 4.3f, 15, 13);

	ss.clear();
}

void SceneEnd::Render()
{
}

void SceneEnd::Exit()
{
	SceneBase::Exit();
}
