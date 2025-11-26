#include "main.h"
#include "scene.h"
#include "gameObject.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "lightManager.h"
#include <algorithm>

bool Scene::InitializeBase() {
	m_isInitialized.store(false, std::memory_order_release);

	// ライトマネージャーは必ずライトの先頭に追加する
	AddGameObject(new LightManager(), TYPE_LIGHT);

	Initialize();

	m_future = std::async(std::launch::async, [this]() { ObjectInitialize(); });

	m_isInitializedBase = true;

	return true;
}

void Scene::FinalizeBase() {
	//非同期完了待ち
	if (m_future.valid()) {
		m_future.wait();
	}

	if (!m_isInitialized.load(std::memory_order_acquire)) {
		return;
	}

	// シーン固有の終了処理
	Finalize();

	// GameObjectの終了処理と解放
	ObjectFinalize();
}

void Scene::UpdateBase(double deltaTime) {
	if (!m_isInitialized.load(std::memory_order_acquire)) {
		return;
	}

	// シーン固有の更新処理
	Update(deltaTime);

	// GameObjectの更新処理
	ObjectUpdate(deltaTime);

	// isDestroyがtrueのGameObjectを削除する
	ObjectDestroy();
}

void Scene::DrawBase() {
	if (!m_isInitialized.load(std::memory_order_acquire)) {
		return;
	}

	// シーン固有の描画処理
	Draw();

	// GameObjectの描画処理
	ObjectDraw();
}

void Scene::CleanUpBase() {
	if (!m_isInitialized.load(std::memory_order_acquire)) {
		return;
	}
	// シーン固有のクリーンアップ処理
	CleanUp();
}

GameObject* Scene::AddGameObject(GameObject* gameObject, OBJECT_TYPE type) {
	gameObject->SetScene(this);
	m_gameObjects[type].push_back(gameObject);
	return gameObject;
}

bool Scene::ObjectInitialize() {
	// GameObjectの初期化
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			if (!gameObject->InitializeBase()) {
				return false;
			}
		}
	}

	m_isInitialized.store(true, std::memory_order_release);

	return true;
}

void Scene::ObjectFinalize() {
	// GameObjectの終了処理と解放
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->Finalize();
			delete gameObject;
		}
		objects.clear();
	}
}

void Scene::ObjectUpdate(double deltaTime) {
	// GameObjectの更新処理
	for (auto& objects : m_gameObjects) {
		for (auto& gameObject : objects) {
			gameObject->UpdateBase(deltaTime);
		}
	}
}

void Scene::ObjectDestroy() {
	//isDestroyがtrueのGameObjectを削除する
	for (auto& objects : m_gameObjects) {
		objects.remove_if([](GameObject* gameObject) {
			return gameObject->IsDestroy();
			});
	}
}

void Scene::ObjectDraw() {
	//レンダーターゲット0に描画
//	RENDERER.SetRenderTarget(0);

	// ライトの描画
	DrawLights();

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
		DrawOpaque(camera);
		DrawCutout(camera);
		DrawTransparent(camera);
	}

	// メインカメラで描画
	if (mainCamera) {
		mainCamera->DrawBase();

		// GameObjectの描画
		DrawOpaque(mainCamera);
		DrawCutout(mainCamera);
		DrawTransparent(mainCamera);
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

void Scene::DrawLights() const {
	//ライトオブジェクトの描画
	for (auto& light : m_gameObjects[TYPE_LIGHT]) {
		light->DrawBase();
		//シャドウキャスターの場合、シャドウマップ作成用にシーンを描画
		Light* lightObj = dynamic_cast<Light*>(light);
		if (lightObj && lightObj->IsShadowCaster() && lightObj->IsActive()) {
			RENDERER.SetRasterizerState(RASTERIZER_MODE::SHADOW);
			for (auto& object : m_gameObjects[TYPE_OPAQUE]) {
				object->DrawShadowBase();
			}
			for (auto& object : m_gameObjects[TYPE_CUTOUT]) {
				object->DrawShadowBase();
			}
		}
	}
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
