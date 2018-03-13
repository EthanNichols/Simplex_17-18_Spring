#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Ethan Nichols - etn6701@rit.edu";

	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 10.0f, ZERO_V3, AXIS_Y);

	//init the camera
	m_pCamera = new MyCamera();
	m_pCamera->SetPositionTargetAndUp(
			vector3(0.0f, 3.0f, 20.0f), //Where my eyes are
			vector3(0.0f, 3.0f, 19.0f), //where what I'm looking at is
			AXIS_Y);					//what is up

	m_pCamera->SetUp(AXIS_Y);

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

	//Get and set a normalized vector3 facing forward from the camera view
	vector3 lForward = m_pCamera->GetTarget() - m_pCamera->GetPosition();
	lForward = lForward / (float)glm::sqrt((lForward.x * lForward.x) + (lForward.y * lForward.y) + (lForward.z * lForward.z));

	//Get the vector3 that is up from the camera
	vector3 lUp = m_pCamera->GetUp();
	lUp = lUp / (float)glm::sqrt((lUp.x * lUp.x) + (lUp.y * lUp.y) + (lUp.z * lUp.z));

	//Get the cross product of the forward and up to get the left vector3 and normalize it
	vector3 lLeft = vector3(((lForward.y * lUp.z) - (lForward.z * lUp.y)),
							((lForward.z * lUp.x) - (lForward.x * lUp.z)),
							((lForward.x * lUp.y) - (lForward.y * lUp.x)));
	lLeft = lLeft / (float)glm::sqrt((lLeft.x * lLeft.x) + (lLeft.y * lLeft.y) + (lLeft.z * lLeft.z));

	//Calculate the rotation of the camera
	glm::quat rotation = glm::angleAxis(m_fAngleY, lUp) * glm::angleAxis(-m_fAngleX, lLeft);

	//Set the new forward and up vectors for the camera
	lForward = rotation * lForward;
	lUp = rotation * lUp;

	//Reset the rotation angle amounts
	m_fAngleX = 0;
	m_fAngleY = 0;

	//Calculate the movement forward and left from the inputs
	vector3 movement = lForward * m_iforward + lLeft * m_ileft;

	//Smoothly (LERP) move to the new position
	vector3 newPos = glm::lerp(m_pCamera->GetPosition(), m_pCamera->GetPosition() + movement, 1.0f);

	//Set the new position
	m_pCamera->SetPosition(newPos);

	//Set the target in front of the camera's relative position
	m_pCamera->SetTarget(m_pCamera->GetPosition() + lForward);

	//Set the current up vector to the previous up vector
	m_pCamera->SetUp(lUp);

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
