#include "main.h"
#include "scene.h"
#include "gameObject.h"
#include "texture.h"
#include "renderer.h"
#include "shaders.h"
#include "camera.h"
#include <algorithm>

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
//	RENDERER.SetRenderTarget(0);

	RENDERER.SetDefaultRenderTarget();

	Camera* mainCamera = nullptr;

	// カメラごとに描画
	for (auto& object : m_gameObjects[TYPE_CAMERA]) {
		auto camera = static_cast<Camera*>(object);
		if (camera->IsMainCamera()) {
			mainCamera = camera;
			continue;
		}
		object->DrawBase();
		// GameObjectの描画
		for (OBJECT_TYPE i = TYPE_NONE; i < TYPE_MAX; i = OBJECT_TYPE(i + 1)) {
			switch (i) {
				case TYPE_OPAQUE:
					DrawOpaque(camera);
					break;
				case TYPE_CUTOUT:
					DrawCutout(camera);
					break;
				case TYPE_TRANSPARENT:
					DrawTransparent(camera);
					break;
				default:
					// それ以外のタイプはここで処理しない
					break;
			}
		}

	}

	// メインカメラで描画
	if (mainCamera) {
		mainCamera->DrawBase();

		// GameObjectの描画
		for (OBJECT_TYPE i = TYPE_NONE; i < TYPE_MAX; i = OBJECT_TYPE(i + 1)) {
			switch (i) {
				case TYPE_OPAQUE:
					DrawOpaque(mainCamera);
					break;
				case TYPE_CUTOUT:
					DrawCutout(mainCamera);
					break;
				case TYPE_TRANSPARENT:
					DrawTransparent(mainCamera);
					break;
				default:
					// それ以外のタイプはここで処理しない
					break;
			}
		}
	}

	//2D行列設定
	RENDERER.Set2DMatrix();

	//深度バッファ無効
	RENDERER.SetDepthStencilState(DEPTH_MODE::DISABLE);


	// エフェクト前のUI描画
	DrawBeforeEffect();
	// ポストプロセス描画
//	DrawPostProcess();
	// エフェクト後のUI描画
//	DrawAfterEffect();

	// レンダーターゲットをデフォルトに戻す
//	RENDERER.SetDefaultRenderTarget();

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

void Scene::DrawOpaque(Camera* camera) const {
	//深度バッファ有効
	RENDERER.SetDepthStencilState(DEPTH_MODE::ENABLE);

	//不透明オブジェクトの描画
	for (auto& gameObject : m_gameObjects[TYPE_OPAQUE]) {
		gameObject->DrawBase();
	}

}

void Scene::DrawCutout(Camera* camera) const {
	//アルファブレンドをカットアウト用に設定
	RENDERER.SetATCEnable(true);
	//カットアウトオブジェクトの描画
	for (auto& gameObject : m_gameObjects[TYPE_CUTOUT]) {
		gameObject->DrawBase();
	}

	//アルファブレンドを元に戻す
	RENDERER.SetATCEnable(false);
}

void Scene::DrawTransparent(Camera* camera) const {
	//深度バッファ読み取り専用
	RENDERER.SetDepthStencilState(DEPTH_MODE::READ_ONLY);

	//カメラの位置
	Vector3 camPos = camera->GetPosition();

	//リストをソートするためのベクター
//	std::vector<GameObject*> transparentObjects(m_gameObjects[TYPE_TRANSPARENT].begin(), m_gameObjects[TYPE_TRANSPARENT].end());

	//カメラからの距離でソート（遠い順）
	//std::sort(transparentObjects.begin(), transparentObjects.end(),
	//	[&](GameObject* a, GameObject* b) {
	//		float distA = (a->GetPosition() - camPos).LengthSquared();
	//		float distB = (b->GetPosition() - camPos).LengthSquared();
	//		return distA > distB; // 遠い順にソート
	//	});

	//透明オブジェクトの描画
	//for (auto& gameObject : transparentObjects) {
	//	gameObject->DrawBase();
	//}

	for (auto& gameObject : m_gameObjects[TYPE_TRANSPARENT]) {
		gameObject->DrawBase();
	}

}

void Scene::DrawBeforeEffect() const {
	//エフェクト前オブジェクトの描画
	for (auto& gameObject : m_gameObjects[TYPE_BEFORE_PROCESS_UI]) {
		gameObject->DrawBase();
	}
}

void Scene::DrawPostProcess() {
	//ポストプロセスオブジェクトの描画
	for (auto& gameObject : m_gameObjects[TYPE_POST_PROCESS]) {
		m_renderTargetIndex = 1 - m_renderTargetIndex;
		RENDERER.SetRenderTarget(m_renderTargetIndex);
		gameObject->DrawBase();
	}
}


void Scene::DrawAfterEffect() const {
	//エフェクト後オブジェクトの描画
	for (auto& gameObject : m_gameObjects[TYPE_AFTER_PROCESS_UI]) {
		gameObject->DrawBase();
	}
}
