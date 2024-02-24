#include "SceneKinematics.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneKinematics::SceneKinematics()
{
}

SceneKinematics::~SceneKinematics()
{
}

void SceneKinematics::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	m_gravity.Set(0, -9.8f, 0); //init gravity as 9.8ms-2 downwards
	Math::InitRNG();

	m_ghost = new GameObject(GameObject::GO_BALL);

	//Exercise 1: construct 10 GameObject with type GO_BALL and add into m_goList
	for (int i = 0; i < 100; ++i)
	{
		m_goList.push_back(new GameObject(GameObject::GO_BALL));
	}
	m_timeGO = NULL;

	// Custom stuffs
	x = y = 0;
	w = h = 0;
	CurrMouseX = CurrMouseY = 0;
	m_ghost_speed = 0;
}

void SceneKinematics::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	// Custom Stuffs
	Application::GetCursorPos(&x, &y);

	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();

	// Calculating mouse world pos
	CurrMouseX = (x / w) * m_worldWidth;
	CurrMouseY = (h - y) / (h / m_worldHeight);

	//Keyboard Section
	if(Application::IsKeyPressed('1'))
		glEnable(GL_CULL_FACE);
	if(Application::IsKeyPressed('2'))
		glDisable(GL_CULL_FACE);
	if(Application::IsKeyPressed('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(Application::IsKeyPressed('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	if(Application::IsKeyPressed(VK_OEM_PLUS))
	{
		//Exercise 6: adjust simulation speed
		m_speed = Math::Min(10.f, m_speed + 10.f * static_cast<float>(dt));
	}
	if(Application::IsKeyPressed(VK_OEM_MINUS))
	{
		//Exercise 6: adjust simulation speed
		m_speed = Math::Max(0.f, m_speed - 10.f * static_cast<float>(dt));
	}
	if(Application::IsKeyPressed('C'))
	{
		//Exercise 9: clear screen
		for (std::vector<GameObject*>::iterator itr = m_goList.begin(); itr != m_goList.end(); ++itr)
		{
			GameObject* go = (GameObject*)*itr;
			go->active = false;
		}
	}
	if(Application::IsKeyPressed(VK_SPACE))
	{
		//Exercise 9: spawn balls
		for (std::vector<GameObject*>::iterator itr = m_goList.begin(); itr != m_goList.end(); ++itr)
		{
			GameObject* go = (GameObject*)*itr;

			if (go->active == false)
			{
				float RandX = Math::RandFloatMinMax(0, m_worldWidth);
				float RandY = Math::RandFloatMinMax(0, m_worldHeight);

				float VelX = Math::RandFloatMinMax(-25, 25);
				float VelY = Math::RandFloatMinMax(-25, 25);

				go->type = GameObject::GO_BALL;
				go->vel.Set(VelX, VelY, 0);
				go->pos.Set(RandX, RandY, 0);
				go->active = true;
			}
		}
	}
	if(Application::IsKeyPressed('V'))
	{
		//Exercise 9: spawn obstacles
		for (std::vector<GameObject*>::iterator itr = m_goList.begin(); itr != m_goList.end(); ++itr)
		{
			GameObject* go = (GameObject*)*itr;
			
			if (go->active == false)
			{
				float RandX = Math::RandFloatMinMax(0, m_worldWidth);
				float RandY = Math::RandFloatMinMax(0, m_worldHeight);

				go->type = GameObject::GO_CUBE;
				go->pos.Set(RandX, RandY, 0);
				go->active = true;
			}
		}
	}

	//Mouse Section
	static bool bLButtonState = false;
	//Exercise 10: ghost code here
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		
		//double x, y;
		//Application::GetCursorPos(&x, &y);
		//int w = Application::GetWindowWidth();
		//int h = Application::GetWindowHeight();

		//Exercise 10: spawn ghost ball
		m_ghost->active = true;
		m_ghost->type = GameObject::GO_BALL;
		m_ghost->pos.Set(CurrMouseX, CurrMouseY, 0);
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;

		/*
		//int w = Application::GetWindowWidth();
		//int h = Application::GetWindowHeight();
		
		//Exercise 4: spawn ball
		//for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		//{
		//	GameObject* go = (GameObject*)*it;

		//	if (go->active == false)
		//	{
		//		double x, y;
		//		Application::GetCursorPos(&x, &y);

		//		go->active = true;
		//		go->type = GameObject::GO_BALL;
		//		//go->pos.Set((x/w) * m_worldWidth, (h - y) / (h / m_worldHeight), 0);	
		//		go->pos.Set(CurrMouseX, CurrMouseY, 0);
		//		go->vel.Set(20, 20, 0);

		//		// DEBUG
		//		//std::cout << "x: " << (x / w) * m_worldWidth << std::endl;
		//		//std::cout << "y: " << (h - y) / (h / m_worldHeight) << std::endl << std::endl;
		//		//std::cout << "W: " << w << std::endl;
		//		//std::cout << "H: " << h << std::endl;

		//		break;
		//	}
		//} 
		*/
						
		//Exercise 10: replace Exercise 4 code and use ghost to determine ball velocity
		Vector3 Dir = m_ghost->pos - Vector3(CurrMouseX, CurrMouseY, m_ghost->pos.z);
		m_ghost->active = false;

		//Exercise 11: kinematics equation
		
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;

			if (go->active == false)
			{
				go->active = true;
				go->type = GameObject::GO_BALL;
				go->pos = m_ghost->pos;
				go->vel = Dir;

				// ----------------- Exercise 11 -----------------------
				m_timeGO = go;
				m_timeTaken1 = m_timeTaken2 = 0;
				m_heightMax = m_timeGO->pos.y;
				m_distanceMax = 0;


				// 11)a)i)     t = (v - u ) / a
				m_timeEstimated1 = (0 - go->vel.y) / m_gravity.y;	// t = (v - u) / a

				// 11)a)ii)	 s = ut + 1/2(a * t^2)  (K3)
				//			 1/2 (a * t^2) + ut - s = 0 
				//		     t = (-u (+-) sqrt( (u^2) - 2as )) / a
				float temp1 = (((-go->vel.y) - sqrt((go->vel.y * go->vel.y) - (2 * m_gravity.y * go->pos.y))) / m_gravity.y);
				float temp2 = (((-go->vel.y) + sqrt((go->vel.y * go->vel.y) - (2 * m_gravity.y * go->pos.y))) / m_gravity.y);

				if (temp1 >= temp2)
					m_timeEstimated2 = temp2;
				else
					m_timeEstimated2 = temp1;

				// 11)a)iii)	v^2 = u^2 + 2as (K4)
				//				s = ( (v^2) - (u^2) )/2a )
				m_heightEstimated = go->pos.y + (((0) - (go->vel.y) * (go->pos.y)) / (2 * (m_gravity.y)));

				// 11)a)iv)		s = ut + 1/2(a * t^2)  (K3)
				m_distanceEstimated = fabs((go->vel.x * m_timeEstimated2) + (m_gravity.x * (m_timeEstimated2 * m_timeEstimated2)));

				// 11)b)i


				// 11) b) ii

				// 11) b) iii

				// 11) b) iv

				// -----------------------------------------------------
			}
		}
	}
	
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
		//Exercise 7: spawn obstacles using GO_CUBE
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();

		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;

			if (go->active == false)
			{
				go->active = true;
				go->type = GameObject::GO_CUBE;
				go->pos.Set(CurrMouseX, CurrMouseY, 0);

				break;
			}
		}
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}

	//Physics Simulation Section
	fps = (float)(1.f / dt);

	//Exercise 11: update kinematics information
	if (m_timeGO && m_timeGO->active)
	{
		if (m_timeGO->vel.y > 0)
		{
			m_timeTaken1 += dt * m_speed;
			m_heightMax = m_timeGO->pos.y;
		}
		if (m_timeGO->pos.y > 0)
		{
			m_timeTaken2 += dt * m_speed;
			m_distanceMax += m_timeGO->vel.x * dt * m_speed;
		}
	}

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;

		if(go->active)
		{
			if(go->type == GameObject::GO_BALL)
			{
				//Exercise 2: implement equation 1 & 2
				go->vel += m_gravity * dt * m_speed;	// dv = a * dt
				go->pos += go->vel * dt * m_speed;		// ds = vel * dt;
				

				//Exercise 12: replace Exercise 2 code and use average speed instead

			}

			//Exercise 8: check collision with GO_CUBE
			if (go->type == GameObject::GO_BALL)
			{
				for (std::vector<GameObject*>::iterator itr2 = m_goList.begin(); itr2 != m_goList.end(); ++itr2)
				{
					GameObject* go_Cube = (GameObject*)*itr2;

					if (go_Cube->type == GameObject::GO_CUBE)
					{
						if (go->active && go_Cube->active)
						{
							// If
							//	(Distance between Ball & Cube) < (Ball radius + Cube radius)
							if ( (go->pos - go_Cube->pos).Length() 
								< go->scale.x + go_Cube->scale.x)
							{
								go->active = false;
								go_Cube->active = false;
							}
						}
					}
				}
			}

			//Exercise 5: unspawn ball when outside window
			if (go->pos.x < 0 || go->pos.x > m_worldWidth ||
				go->pos.y < 0 || go->pos.y > m_worldHeight)
			{		
				go->active = false;
			}
		}
	}
}

void SceneKinematics::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
		//Exercise 3: render a sphere with radius 1
		modelStack.PushMatrix();
			modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
			modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
			RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();

		break;
	case GameObject::GO_CUBE:
		//Exercise 7: render a cube with length 2
		modelStack.PushMatrix();
			modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
			modelStack.Scale(go->scale.x * 2, go->scale.y * 2, go->scale.z * 2);
			RenderMesh(meshList[GEO_CUBE], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneKinematics::Render()
{
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
	
	RenderMesh(meshList[GEO_AXES], false);

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
	}
	if(m_ghost->active)
	{
		RenderGO(m_ghost);
	}

	//On screen text
	std::ostringstream ss;
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);
	
	//Exercise 6: print simulation speed
	ss.str("");
	ss.precision(5);
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 5);
	
	//Exercise 10: print m_ghost position and velocity information
	ss.str("");
	ss.precision(5);
	ss << "Ghost Pos: " << m_ghost->pos;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 7);

	ss.str("");
	ss.precision(5);
	ss << "Ghost Vel: " << m_ghost_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 9);

	//Exercise 11: print kinematics information
	ss.str("");
	ss << "Est Time 1: " << m_timeEstimated1;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 11);

	ss.str("");
	ss << "Est Time 2: " << m_timeEstimated2;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 13);

	ss.str("");
	ss << "Est Max Height: " << m_heightEstimated;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 15);

	ss.str("");
	ss << "Est Hor Dist: " << m_distanceEstimated;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 18);

	ss.str("");
	ss << "TimeTaken 1: " << m_timeTaken1;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 21);

	ss.str("");
	ss << "TimeTaken 2: " << m_timeTaken2;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 24);

	ss.str("");
	ss << "Max height: " << m_heightMax;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 27);

	ss.str("");
	ss << "Max Hor Dist: " << m_distanceMax;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 30);

	RenderTextOnScreen(meshList[GEO_TEXT], "Kinematics", Color(0, 1, 0), 3, 0, 0);
}

void SceneKinematics::Exit()
{
	// Cleanup VBO
	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if(meshList[i])
			delete meshList[i];
	}
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
	
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}
