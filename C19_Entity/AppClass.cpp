#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUp(vector3(0.0f, 3.0f, 13.0f), //Position
		vector3(0.0f, 3.0f, 12.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

	//Set the instance of the enitity manager
	entityManager = new _MyEntityManager();

	//Add/create entites in the entity manager
	entityManager->AddEntity("Minecraft\\Creeper.obj", "Creeper");
	entityManager->AddEntity("Minecraft\\Steve.obj", "Steve");
	entityManager->AddEntity("Minecraft\\Cow.obj", "Cow");
	entityManager->AddEntity("Minecraft\\Zombie.obj", "Zombie");
	entityManager->AddEntity("Minecraft\\Pig.obj", "Pig");

	/*
	//creeper
	m_pCreeper = new MyEntity("Minecraft\\Creeper.obj", "Creeper");

	//steve
	m_pSteve = new MyEntity("Minecraft\\Steve.obj", "Steve");

	m_pCow = new MyEntity("Minecraft\\Cow.obj", "Cow");
	m_pZombie = new MyEntity("Minecraft\\Zombie.obj", "Zombie");
	m_pPig = new MyEntity("Minecraft\\Pig.obj", "Pig");
	*/
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Set model matrix to the creeper
	matrix4 mCreeper = glm::translate(m_v3Creeper) * ToMatrix4(m_qCreeper) * ToMatrix4(m_qArcBall);
	//m_pCreeper->SetModelMatrix(mCreeper);


	//Set model matrix to Steve
	matrix4 mSteve = glm::translate(vector3(2.25f, 0.0f, 0.0f)) * glm::rotate(IDENTITY_M4, -55.0f, AXIS_Z);
	//m_pSteve->SetModelMatrix(mSteve);

	matrix4 mCow = glm::translate(vector3(1.55f, 1.0f, 0.0f)) * glm::rotate(IDENTITY_M4, -55.0f, AXIS_Z);
	matrix4 mPig = glm::translate(vector3(0.0f, 0.5f, -1.5f)) * glm::rotate(IDENTITY_M4, -55.0f, AXIS_Z);
	matrix4 mZombie = glm::translate(vector3(1.55f, 0.0f, -3.0f)) * glm::rotate(IDENTITY_M4, -55.0f, AXIS_Z);


	//m_pCow->SetModelMatrix(mCow);
	//m_pPig->SetModelMatrix(mPig);
	//m_pZombie->SetModelMatrix(mZombie);

	//Set all of the matricx models for the entities
	entityManager->SetModelMatrix(mCreeper, "Creeper");
	entityManager->SetModelMatrix(mSteve, "Steve");
	entityManager->SetModelMatrix(mCow, "Cow");
	entityManager->SetModelMatrix(mZombie, "Zombie");
	entityManager->SetModelMatrix(mPig, "Pig");

	//Check collision
	//bool bColliding = m_pCreeper->IsColliding(m_pSteve);

	//Update and test for collisions
	entityManager->Update();

	//Add all of the entites to the render list to be drawn
	entityManager->AddToRenderList("Creeper", true);
	entityManager->AddToRenderList("Steve", true);
	entityManager->AddToRenderList("Cow", true);
	entityManager->AddToRenderList("Zombie", true);
	entityManager->AddToRenderList("Pig", true);

	/*
	//Add objects to render list
	m_pCreeper->AddToRenderList(true);
	m_pSteve->AddToRenderList(true);
	m_pZombie->AddToRenderList(true);
	m_pPig->AddToRenderList(true);
	m_pCow->AddToRenderList(true);
	*/

}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}

void Application::Release(void)
{
	//release the creeper
	SafeDelete(m_pCreeper);

	//release Steve
	SafeDelete(m_pSteve);

	SafeDelete(m_pZombie);
	SafeDelete(m_pCow);
	SafeDelete(m_pPig);

	//release GUI
	ShutdownGUI();
}
