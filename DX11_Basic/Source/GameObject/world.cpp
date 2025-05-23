#include "main.h"
#include "world.h"
#include "gameObject.h"

bool World::Initialize() {
	// 追加待ちのGameObjectを追加する
	SetGameObject();

	// GameObjectの初期化
	for (auto gameObject : m_gameObjects) {
		if (!gameObject->Initialize()) {
			return false;
		}
	}

	return true;
}

void World::Finalize() {
	for (auto gameObject : m_gameObjects) {
		gameObject->Finalize();
		delete gameObject;
	}

	m_gameObjects.clear();
}

void World::Update(double deltaTime) {
	for (auto gameObject : m_gameObjects) {
		gameObject->Update(deltaTime);
	}

	//オブジェクトタイプ順にソート
	m_gameObjects.sort([](const GameObject* a, const GameObject* b) {return a->GetType() < b->GetType(); });
}

void World::Draw() const {
	for (auto gameObject : m_gameObjects) {
		gameObject->Draw();
	}
}

void World::CleanUp() {
	//isDestroyがtrueのGameObjectを削除する
	for (auto it = m_gameObjects.begin(); it != m_gameObjects.end();) {
		if ((*it)->IsDestroy()) {
			delete *it;
			it = m_gameObjects.erase(it);
		}
		else {
			++it;
		}
	}

	SetGameObject();
}

void World::AddGameObject(GameObject* gameObject) {
	gameObject->SetWorld(this);
	m_gameObjectsAddList.push_back(gameObject);
}

void World::SetGameObject() {
	//追加待ちのGameObjectを追加する
	for (auto lisObject : m_gameObjectsAddList) {
		m_gameObjects.push_back(lisObject);
	}
	//追加待ちだったオブジェクトを初期化
	for (auto obj : m_gameObjectsAddList) {
		obj->Initialize();
	}

	m_gameObjectsAddList.clear();
}
