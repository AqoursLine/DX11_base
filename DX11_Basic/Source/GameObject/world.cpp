#include "main.h"
#include "world.h"
#include "gameObject.h"

bool World::Initialize() {
	// GameObject‚Ì‰Šú‰»
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			if (!gameObject->Initialize()) {
				return false;
			}
		}
	}

	return true;
}

void World::Finalize() {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->Finalize();
			delete gameObject;
		}
		objects.clear();
	}
}

void World::Update(double deltaTime) {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->Update(deltaTime);
		}
	}
}

void World::Draw() const {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->Draw();
		}
	}
}

void World::CleanUp() {
	//isDestroy‚ªtrue‚ÌGameObject‚ðíœ‚·‚é
	for (auto& objects : m_gameObjects) {
		objects.remove_if([](GameObject* gameObject) {
			return gameObject->IsDestroy();
			});
	}
}

GameObject* World::AddGameObject(GameObject* gameObject, OBJECT_TYPE type) {
	gameObject->SetWorld(this);
	gameObject->Initialize();
	m_gameObjects[type].push_back(gameObject);

	return gameObject;
}

