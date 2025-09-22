#include "main.h"
#include "scene.h"
#include "gameObject.h"
#include "texture.h"

bool Scene::Initialize() {
	// GameObjectの初期化
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

	Texture::ReleaseAll(); // テクスチャのキャッシュを解放
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
	//isDestroyがtrueのGameObjectを削除する
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

