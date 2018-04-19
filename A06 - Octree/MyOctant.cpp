#include "MyOctant.h"

using namespace Simplex;

//Static variables
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 0;
uint MyOctant::m_uIdealEntityCount = 0;

MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//Initialize
	Init();

	//Intialize variables
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_Octant = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax;

	//Get the amount of entities in the scene
	uint uEntities = m_pEntityMngr->GetEntityCount();

	//Add all of the entities and rigidbodies to the respective vectors
	for (uint i = 0; i < uEntities; i++)
	{
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();
		lMinMax.push_back(pRigidBody->GetMinGlobal());
		lMinMax.push_back(pRigidBody->GetMaxGlobal());
	}

	MyRigidBody* pRigidbody = new MyRigidBody(lMinMax);

	//Get half the width of the rigid body
	vector3 v3HalfWidth = pRigidbody->GetHalfWidth();

	float fMax = v3HalfWidth.x;

	for (int i = 1; i < 3; i++)
		if (fMax < v3HalfWidth[i])
			fMax = v3HalfWidth[i];
		
	vector3 v3Center = pRigidbody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidbody);

	m_fSize = fMax * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();

	//Get the center and size of the octants
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//Calc the min and max vecotrs
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	//Increate the amount of octants
	m_uOctantCount++;
}

MyOctant::MyOctant(MyOctant const & other)
{
	//Swap the values with 'other' values
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_Octant, other.m_Octant;
	m_lChild, other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++)
		m_pChild[i] = other.m_pChild[i];
}

MyOctant::~MyOctant()
{
	Release();
}

void MyOctant::Swap(MyOctant & other)
{
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_uID, other.m_uID);
	std::swap(m_Octant, other.m_Octant);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	//Get the mesh and entity managers
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (uint i = 0; i < 8; i++)
	for (uint i = 0; i < 8; i++)
		std::swap(m_pChild[i], other.m_pChild[i]);
}

//Return the size of the octant
float MyOctant::GetSize() { return m_fSize; }

//Return position of min / max
vector3 MyOctant::GetMinGlobal() { return m_v3Min; }
vector3 MyOctant::GetMaxGlobal() { return m_v3Max; }

//Return if there are no children in the octant
bool MyOctant::IsLeaf() { return m_uChildren == 0; }

//Return the count of octants in the scene
uint MyOctant::GetOctantCount() { return m_uOctantCount; }

//AARB Collision check
bool MyOctant::IsColliding(uint a_uRBIndex)
{

	//Get the amount of entities
	uint uEntities = m_pEntityMngr->GetEntityCount();

	//Check if index is larger than the number of 
	//entities. If so, no collision
	if (a_uRBIndex >= uEntities) { return false; }

	//AARB Collision: grabs vectors in global space
	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3MinO = pRigidBody->GetMinGlobal();
	vector3 v3MaxO = pRigidBody->GetMaxGlobal();

	//X-axis
	if (m_v3Max.x < v3MinO.x) { return false; }
	if (m_v3Min.x > v3MaxO.x) { return false; }

	//Y-axis
	if (m_v3Max.y < v3MinO.y) { return false; }
	if (m_v3Min.y > v3MaxO.y) { return false; }

	//Z-axis
	if (m_v3Max.z < v3MinO.z) { return false; }
	if (m_v3Min.z > v3MaxO.z) { return false; }

	//Return that there is a collision
	return true;
}

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	//Test if the octant should be displayed, and add it to the render list
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}

	//Test all of the children of the octant if it idn't displayed
	for (uint nIndex = 0; nIndex < m_uChildren; nIndex++)
		m_pChild[nIndex]->Display(a_nIndex);
}

void MyOctant::Display(vector3 a_v3Color)
{
	//Loop through all of the children of the octant
	for (uint i = 0; i < m_uChildren; i++)
		m_pChild[i]->Display(a_v3Color);

	//Display the octant
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}


void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	//Loop through all of the leafs of the octant
	uint nLeafs = m_lChild.size();
	for (uint nChild = 0; nChild < nLeafs; nChild++)
		m_lChild[nChild]->DisplayLeafs(a_v3Color);

	//Display the octant
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::ClearEntityList()
{
	//Loop through and remove entities from the octants
	for (uint nChild = 0; nChild < m_uChildren; nChild++)
		m_lChild[nChild]->ClearEntityList();

	//Clear the entity list
	m_EntityList.clear();
}

void MyOctant::Subdivide()
{
	//If node is greater than or at max level, return nothing
	if (m_uLevel >= m_uMaxLevel) { return; }

	//If node is already subdivided, return nothing
	if (m_uChildren != 0) { return; }

	//Not a leaf due to having children
	m_uChildren = 8;

	float fSize = m_fSize / 4.0f;
	float fsizeD = fSize * 2.0f;
	vector3 v3Center;

	//Divide the 8 internal cubes to make an octant
	//Bottom Left Back Cube
	v3Center = m_v3Center;
	v3Center.x -= fSize;
	v3Center.y -= fSize;
	v3Center.z -= fSize;
	m_pChild[0] = new MyOctant(v3Center, fsizeD);

	//Bottom Right Back Cube
	v3Center.x += fsizeD;
	m_pChild[1] = new MyOctant(v3Center, fsizeD);

	//Bottom Right Front Cube
	v3Center.z += fsizeD;
	m_pChild[2] = new MyOctant(v3Center, fsizeD);

	//Bottom Left Front Cube
	v3Center.x -= fsizeD;
	m_pChild[3] = new MyOctant(v3Center, fsizeD);

	//Top Left Front Cube
	v3Center.y += fsizeD;
	m_pChild[4] = new MyOctant(v3Center, fsizeD);

	//Top Left Back Cube
	v3Center.z -= fsizeD;
	m_pChild[5] = new MyOctant(v3Center, fsizeD);

	//Top Right Back Cube
	v3Center.x += fsizeD;
	m_pChild[6] = new MyOctant(v3Center, fsizeD);

	//Top Right Front Cube
	v3Center.z += fsizeD;
	m_pChild[7] = new MyOctant(v3Center, fsizeD);

	for (uint nIndex = 0; nIndex < 8; nIndex++)
	{
		m_pChild[nIndex]->m_Octant = m_Octant;
		m_pChild[nIndex]->m_pParent = this;
		m_pChild[nIndex]->m_uLevel = m_uLevel + 1;

		if (m_pChild[nIndex]->ContainsMoreThan(m_uIdealEntityCount))
			m_pChild[nIndex]->Subdivide();
	}
}

bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	uint nCount = 0;
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	for (uint n = 0; n < nObjectCount; n++)
	{
		if (IsColliding(n))
			nCount++;

		if (nCount > a_nEntities)
			return true;
	}
	return false;
}

void MyOctant::KillBranches()
{
	//Recursive method: goes through the entire Octree until it reaches a node 
	//with no children. It'll then return back to the node that called it
	//and kill all the children in the process.
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//If it is not the root, return
	if (m_uLevel != 0) { return; }

	m_uMaxLevel = a_nMaxLevel;

	m_uOctantCount = 1;

	m_EntityList.clear();

	//Empty the tree
	KillBranches();
	m_lChild.clear();

	//If the base tree contains more than the ideal count, then subdivide
	if (ContainsMoreThan(m_uIdealEntityCount))
		Subdivide();

	//Assign octant IDs
	AssignIDtoEntity();

	//Construct object list
	ConstructList();
}

void MyOctant::AssignIDtoEntity()
{
	//Move through the tree until reaching a leaf
	for (uint nChild = 0; nChild < m_uChildren; nChild++)
		m_pChild[nChild]->AssignIDtoEntity();

	//If IsLeaf is true
	if (IsLeaf())
	{
		//Get all the entities
		uint nEntities = m_pEntityMngr->GetEntityCount();

		//Loop through the entities
		for (uint nIndex = 0; nIndex < nEntities; nIndex++)
		{
			
			//Check if the entity is colliding with the leaf
			//Add the entitiy index to the entity
			if (IsColliding(nIndex))
			{
				m_EntityList.push_back(nIndex);
				m_pEntityMngr->AddDimension(nIndex, m_uID);
			}
		}
	}
}

void MyOctant::Release()
{
	if (m_uLevel == 0) { KillBranches(); }

	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

/// Set the initial values for the oct tree
void MyOctant::Init()
{
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_Octant = nullptr;
	m_pParent = nullptr;

	for (uint n = 0; n < 8; n++)
		m_pChild[n] = nullptr;
}

void MyOctant::ConstructList()
{
	for (uint nChild = 0; nChild < m_uChildren; nChild++)
		m_pChild[nChild]->ConstructList();

	if (m_EntityList.size() > 0)
		m_Octant->m_lChild.push_back(this);
}