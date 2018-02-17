#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//List of vertices for the base circle
	std::vector<vector3> vertices;

	//Top and bottom center vertices
	vector3 topVertex = vector3(0, a_fHeight / 2, 0);
	vector3 bottomVertex = vector3(0, -a_fHeight / 2, 0);

	//Calulcate the degrees each point is spaced apart
	float degree = 360 / (float)a_nSubdivisions;

	//Run through the amount of base vertices and add the positions to the vertices list
	for (int i = 0; i < a_nSubdivisions; i++) {
		vertices.push_back(vector3(a_fRadius * sin(glm::radians(degree * i)), -a_fHeight / 2, a_fRadius * cos(glm::radians(degree * i))));
	}

	//Loop through all of the vertices
	for (int i = 0; i < a_nSubdivisions; i++) {

		//Get the next vertex and make sure it stays in the range of the list
		int next = (i+1 < a_nSubdivisions) ? i+1 : (i+1) - a_nSubdivisions;

		//Create a tri from the base vertices to the top and bottom vertices
		AddTri(vertices[i], vertices[next], topVertex);
		AddTri(vertices[next], vertices[i], bottomVertex);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();
	
	//List of vertices for the base circle
	std::vector<vector3> vertices;

	//Top and bottom center vertices
	vector3 topVertex = vector3(0, a_fHeight / 2, 0);
	vector3 bottomVertex = vector3(0, -a_fHeight / 2, 0);

	//Calulcate the degrees each point is spaced apart
	float degree = 360 / (float)a_nSubdivisions;

	//Run through the amount of base vertices and add the positions to the vertices list
	for (int i = 0; i < a_nSubdivisions; i++) {
		vertices.push_back(vector3(a_fRadius * sin(glm::radians(degree * i)), a_fHeight / 2, a_fRadius * cos(glm::radians(degree * i))));
		vertices.push_back(vector3(a_fRadius * sin(glm::radians(degree * i)), -a_fHeight / 2, a_fRadius * cos(glm::radians(degree * i))));
	}

	int subCount = a_nSubdivisions * 2;

	//Loop through all of the vertices
	for (int i = 0; i < subCount; i+=2) {

		//Get the next vertex and make sure it stays in the range of the list
		int i1 = (i + 2 < subCount) ? i + 2 : (i + 2) - subCount;
		int i2 = (i + 1 < subCount) ? i + 1 : (i + 1) - subCount;
		int i3 = (i + 3 < subCount) ? i + 3 : (i + 3) - subCount;

		//Create a tri from the base vertices to the top and bottom vertices
		AddTri(vertices[i], vertices[i1], topVertex);
		AddTri(vertices[i3], vertices[i2], bottomVertex);
		AddQuad(vertices[i2], vertices[i3], vertices[i], vertices[i1]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//List of vertices for the base circle
	std::vector<vector3> topVertices;
	std::vector<vector3> bottomVertices;

	//Calulcate the degrees each point is spaced apart
	float degree = 360 / (float)a_nSubdivisions;

	//Create the vertices to store in the respective list
	for (int i = 0; i < a_nSubdivisions; i++) {

		//Create the top vertices
		topVertices.push_back(vector3(a_fInnerRadius * sin(glm::radians(degree * i)), a_fHeight / 2, a_fInnerRadius * cos(glm::radians(degree * i))));
		topVertices.push_back(vector3(a_fOuterRadius * sin(glm::radians(degree * i)), a_fHeight / 2, a_fOuterRadius * cos(glm::radians(degree * i))));

		//Create the bottom vertices
		bottomVertices.push_back(vector3(a_fInnerRadius * sin(glm::radians(degree * i)), -a_fHeight / 2, a_fInnerRadius * cos(glm::radians(degree * i))));
		bottomVertices.push_back(vector3(a_fOuterRadius* sin(glm::radians(degree * i)), -a_fHeight / 2, a_fOuterRadius * cos(glm::radians(degree * i))));
	}

	int subCount = a_nSubdivisions * 2;

	//Loop through all of the vertices
	for (int i = 0; i < subCount; i+=2) {

		//Other vertices needed for quads
		int i1 = (i + 1 < subCount) ? i + 1 : (i + 1) - subCount;
		int i2 = (i + 2 < subCount) ? i + 2 : (i + 2) - subCount;
		int i3 = (i + 3 < subCount) ? i + 3 : (i + 3) - subCount;

		//Create all the possible quads to the vertices
		AddQuad(topVertices[i], topVertices[i1], topVertices[i2], topVertices[i3]);
		AddQuad(bottomVertices[i2], bottomVertices[i3], bottomVertices[i], bottomVertices[i1]);
		AddQuad(bottomVertices[i], topVertices[i], bottomVertices[i2], topVertices[i2]);
		AddQuad(bottomVertices[i3], topVertices[i3], bottomVertices[i1], topVertices[i1]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	//Structure for the triangles
	struct faceVerts
	{
		//The vertices to make the triangle
		vector3 v1, v2, v3;

		//Create the face
		faceVerts(vector3 _v1, vector3 _v2, vector3 _v3) {
			v1 = _v1;
			v2 = _v2;
			v3 = _v3;
		}
	};

	//List of vertices for the vertices and faces
	std::vector<vector3> vertices;
	std::vector<faceVerts> faces;

	//Unit Sphere size
	float t = (1 + (float)glm::sqrt(5.0)) / 2.0f;

	//Create the 12 base vertices
	for (int i = 0; i < 12; i++) {

		//Set the golden rectangle offset, and unit size offset
		float localT = (i % 4 < 2) ? t : t * -1;
		int offset = (i % 2 == 0) ? -1 : 1;

		//Calculate the length from the center
		float length = glm::sqrt(1 + localT * localT);

		//Set the vertices for the 3 golden rectangles
		if (i < 4)
			vertices.push_back((vector3(offset, localT, 0) / length) * a_fRadius);
		else if (i < 8)
			vertices.push_back((vector3(0, offset, localT) / length) * a_fRadius);
		else
			vertices.push_back((vector3(localT, 0, offset) / length) * a_fRadius);
	}

//Create the faces for the smallest possible icosphere
#pragma region Manual creation of starting faces
	faces.push_back(faceVerts(vertices[0], vertices[11], vertices[5]));
	faces.push_back(faceVerts(vertices[0], vertices[5], vertices[1]));
	faces.push_back(faceVerts(vertices[0], vertices[1], vertices[7]));
	faces.push_back(faceVerts(vertices[0], vertices[7], vertices[10]));
	faces.push_back(faceVerts(vertices[0], vertices[10], vertices[11]));

	faces.push_back(faceVerts(vertices[3], vertices[9], vertices[4]));
	faces.push_back(faceVerts(vertices[3], vertices[4], vertices[2]));
	faces.push_back(faceVerts(vertices[3], vertices[2], vertices[6]));
	faces.push_back(faceVerts(vertices[3], vertices[6], vertices[8]));
	faces.push_back(faceVerts(vertices[3], vertices[8], vertices[9]));

	faces.push_back(faceVerts(vertices[1], vertices[5], vertices[9]));
	faces.push_back(faceVerts(vertices[5], vertices[11], vertices[4]));
	faces.push_back(faceVerts(vertices[11], vertices[10], vertices[2]));
	faces.push_back(faceVerts(vertices[10], vertices[7], vertices[6]));
	faces.push_back(faceVerts(vertices[7], vertices[1], vertices[8]));

	faces.push_back(faceVerts(vertices[4], vertices[9], vertices[5]));
	faces.push_back(faceVerts(vertices[2], vertices[4], vertices[11]));
	faces.push_back(faceVerts(vertices[6], vertices[2], vertices[10]));
	faces.push_back(faceVerts(vertices[8], vertices[6], vertices[7]));
	faces.push_back(faceVerts(vertices[9], vertices[8], vertices[1]));
#pragma endregion

	//Subdivide the bade triangles
	for (int i = 1; i < a_nSubdivisions; i++) {
		std::vector<faceVerts> newFaces;

		for each (faceVerts tri in faces)
		{
			//Get the three subdivided positions
			vector3 sub1 = (tri.v1 + tri.v2) / 2.0f;
			vector3 sub2 = (tri.v2 + tri.v3) / 2.0f;
			vector3 sub3 = (tri.v3 + tri.v1) / 2.0f;

			//Calculate the length of the positions
			float sub1Length = glm::sqrt(sub1.x * sub1.x + sub1.y * sub1.y + sub1.z * sub1.z);
			float sub2Length = glm::sqrt(sub2.x * sub2.x + sub2.y * sub2.y + sub2.z * sub2.z);
			float sub3Length = glm::sqrt(sub3.x * sub3.x + sub3.y * sub3.y + sub3.z * sub3.z);

			//Normalize and set the actual position for the subdivisions
			sub1 = (sub1 / sub1Length) * a_fRadius;
			sub2 = (sub2 / sub2Length) * a_fRadius;
			sub3 = (sub3 / sub3Length) * a_fRadius;

			//Add the new faces to the local face list
			newFaces.push_back(faceVerts(tri.v1, sub1, sub3));
			newFaces.push_back(faceVerts(tri.v2, sub2, sub1));
			newFaces.push_back(faceVerts(tri.v3, sub3, sub2));
			newFaces.push_back(faceVerts(sub1, sub2, sub3));
		}

		//Clear and set the old face list to the new face list
		faces.clear();
		faces = newFaces;
		newFaces.clear();
	}

	//Create triangles based off of the face list
	for (int i = 0; i < faces.size(); i++) {
		AddTri(faces[i].v1, faces[i].v2, faces[i].v3);
	}

	faces.clear();

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}