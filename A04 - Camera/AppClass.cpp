#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Alberto Bobadilla - labigm@rit.edu";

	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 10.0f, ZERO_V3, AXIS_Y);

	//init the camera
	m_pCamera = new MyCamera();
	m_pCamera->SetPositionTargetAndUp(
			vector3(0.0f, 3.0f, 20.0f), //Where my eyes are
			vector3(0.0f, 3.0f, 19.0f), //where what I'm looking at is
			AXIS_Y);					//what is up

	//Get the singleton
	m_pMyMeshMngr = MyMeshManager::GetInstance();
	m_pMyMeshMngr->SetCamera(m_pCamera);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	vector3 lForward = m_pCamera->GetTarget() - m_pCamera->GetPosition();
	lForward = lForward / (float)glm::sqrt((lForward.x * lForward.x) + (lForward.y * lForward.y) + (lForward.z * lForward.z));
	vector3 lUp = m_pCamera->GetUp();

	vector3 lLeft = vector3(((lForward.y * lUp.z) - (lForward.z * lUp.y)) * 1,
							((lForward.z * lUp.x) - (lForward.x * lUp.z)) * 1,
							((lForward.x * lUp.y) - (lForward.y * lUp.x)) * 1);
	lLeft = lLeft / (float)glm::sqrt((lLeft.x * lLeft.x) + (lLeft.y * lLeft.y) + (lLeft.z * lLeft.z));

	std::cout << lLeft.x << " " << lLeft.y << " " << lLeft.z << " " << std::endl;

	vector3 movement = m_iforward * m_ileft + lLeft * m_ileft;

	m_pCamera->SetPosition(m_pCamera->GetPosition() + lForward * m_iforward + lLeft * m_ileft);
	m_pCamera->SetTarget(m_pCamera->GetTarget() + lForward * m_iforward + lLeft * m_ileft);

	m_iforward = 0;
	m_ileft = 0;

	//Add objects to the Manager
	for (int j = -50; j < 50; j += 2)
	{
		for (int i = -50; i < 50; i += 2)
		{
			m_pMyMeshMngr->AddConeToRenderList(glm::translate(vector3(i, 0.0f, j)));
		}
	}
}
void Application::Display(void)
{
	//Clear the screen
	ClearScreen();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//Render the list of MyMeshManager
	m_pMyMeshMngr->Render();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the MyMeshManager list
	m_pMyMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release the singleton
	MyMeshManager::ReleaseInstance();

	//release the camera
	SafeDelete(m_pCamera);

	//release GUI
	ShutdownGUI();
}
