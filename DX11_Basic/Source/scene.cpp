#include "main.h"
#include "scene.h"
#include "gameObject.h"

bool Scene::Initialize() {
	// GameObject‚Ì‰Šú‰»
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			if (!gameObject->InitializeBase()) {
				return false;
			}
		}
	}

	return true;
}

void Scene::Finalize() {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->Finalize();
			delete gameObject;
		}
		objects.clear();
	}
}

void Scene::Update(double deltaTime) {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->UpdateBase(deltaTime);
		}
	}
}

void Scene::Draw() {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->DrawBase();
		}
	}
}

void Scene::CleanUp() {
	//isDestroy‚ªtrue‚ÌGameObject‚ðíœ‚·‚é
	for (auto& objects : m_gameObjects) {
		objects.remove_if([](GameObject* gameObject) {
			return gameObject->IsDestroy();
			});
	}
}

GameObject* Scene::AddGameObject(GameObject* gameObject, OBJECT_TYPE type) {
	m_gameObjects[type].push_back(gameObject);

	return gameObject;
}

