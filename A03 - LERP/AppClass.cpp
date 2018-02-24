#include "AppClass.h"
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Ethan Nichols - etn6701@rit.edu";
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	//if there are no segments create 7
	if(m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
		This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
		fSize += 0.5f; //increment the size for the next orbit
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 90.0f, AXIS_Z);

	//Timer information
	static float fTimer = 0;
	static uint uClock = m_pSystem->GenClock();
	fTimer += m_pSystem->GetDeltaTime(uClock);

	//Information about the vertices visited, and the circle offset
	static float localPos = 0;
	int pointOffset = 3;

	// draw a shapes
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		//The time between points, and the current percentage between points
		float fTimeBetweenStops = 0.2f;

		//The angle between vertices, and the current and next vertex
		float angleDegree = 360 / (i + pointOffset);
		int currentPoint = (int)(fTimer / fTimeBetweenStops) % (i + pointOffset);
		int nextPoint = (int)((fTimer + fTimeBetweenStops) / fTimeBetweenStops) % (i + pointOffset);

		//Get the percentage between the vertices
		float fPercent = MapValue(fTimer - (fTimeBetweenStops * localPos), 0.0f, fTimeBetweenStops, 0.0f, 1.0f);
		if (fPercent >= 1) {
			localPos++;
			fPercent = MapValue(fTimer - (fTimeBetweenStops * localPos), 0.0f, fTimeBetweenStops, 0.0f, 1.0f);
		}

		//Render the torus
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 90.0f, AXIS_X));

		//Calculate the current and next positions
		vector3 v3CurrentPos = vector3((1 + (i * .5)) * cos(glm::radians(angleDegree * currentPoint)), (1 + (i * .5)) * sin(glm::radians(angleDegree * currentPoint)), 0);
		vector3 v3NextPos = vector3((1 + (i * .5)) * cos(glm::radians(angleDegree * nextPoint)), (1 + (i * .5)) * sin(glm::radians(angleDegree * nextPoint)), 0);
		
		//Lerp between the points and translate the sphere
		vector3 v3LerpVec = glm::lerp(v3CurrentPos, v3NextPos, fPercent);
		matrix4 m4Model = glm::translate(m4Offset, v3LerpVec);

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

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
	//release GUI
	ShutdownGUI();
}