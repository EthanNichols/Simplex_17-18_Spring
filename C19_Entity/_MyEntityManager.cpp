#include "_MyEntityManager.h"



_MyEntityManager::_MyEntityManager()
{
}


_MyEntityManager::~_MyEntityManager()
{
}

void _MyEntityManager::AddEntity(char* fileName, char* id) {

	//Create a new entity with the object file location and id name
	Simplex::MyEntity* entity = new Simplex::MyEntity(fileName, id);

	//Add the entity to the map with the id as the key
	entities.insert(std::pair<char*, Simplex::MyEntity*>(id, entity));
}

void _MyEntityManager::SetModelMatrix(Simplex::matrix4 matToWorld, char* id) {

	//Find and set the model of the entity
	entities.find(id)->second->SetModelMatrix(matToWorld);
}

void _MyEntityManager::RemoveEntity(char* id) {

	//Return if there are no entities to remove
	if (entities.size() == 0) return;

	//Iterator through the map
	std::map<char*, Simplex::MyEntity*>::iterator it;

	//Find the entity in the map and remove it
	it = entities.find(id);
	if (it != entities.end()) {
		entities.erase(it);
	}
}

Simplex::MyEntity _MyEntityManager::GetEntity(char* id) {

	//Return if there are no entities in the map
	if (entities.size() == 0) return NULL;

	//Return the entity with the given key from the map
	return *(entities.find(id)->second);
}

void _MyEntityManager::AddToRenderList(char* id, bool rigidBody) {

	//Add the entity to the render list with/without a rigidbody
	entities.find(id)->second->AddToRenderList(rigidBody);
}

void _MyEntityManager::Update() {

	//Two iterators through the map
	std::map<char*, Simplex::MyEntity*>::iterator it1;
	std::map<char*, Simplex::MyEntity*>::iterator it2;

	//Start one at the beginning and the other one step in front
	it1 = entities.begin();
	it2 = it1++;

	//Loop until the first iterator hits the end
	while (it1 != entities.end()) {

		//if the second iterator hit the end test the next entity
		if (it2 == entities.end()) {
			it1++;
			it2 = it1++;
			continue;
		}

		//Test the first entity is colliding with another entity
		it1->second->IsColliding(it2->second);

		//Step through the map
		it2++;
	}
}