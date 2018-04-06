#pragma once

#include <map>

#include "MyEntity.h"

class _MyEntityManager
{
public:
	_MyEntityManager();
	~_MyEntityManager();

	std::map<char*, Simplex::MyEntity*> entities;

	void AddEntity(char* fileName, char* id);
	void RemoveEntity(char* id);
	Simplex::MyEntity GetEntity(char* id);
	void SetModelMatrix(Simplex::matrix4 matToWorld, char* id);
	void AddToRenderList(char* id, bool rigidBody);
	void Update();
};

