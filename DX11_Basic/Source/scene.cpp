#include "main.h"
#include "scene.h"
#include "gameObject.h"
#include "texture.h"
#include "renderer.h"
#include "shaders.h"

bool Scene::Initialize() {
	//レンダーターゲットの追加
	RENDERER.AddRenderTarget(SCREEN_WIDTH, SCREEN_HEIGHT);

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

	VertexShader::ReleaseAll(); // 頂点シェーダーのキャッシュを解放
	PixelShader::ReleaseAll(); // ピクセルシェーダーのキャッシュを解放
}

void Scene::Update(double deltaTime) {
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->UpdateBase(deltaTime);
		}
	}
}

void Scene::Draw() {
	//レンダーターゲット0に描画
	RENDERER.SetRenderTarget(0);

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

