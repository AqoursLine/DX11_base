#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "testScene.h"
#include "titleScene.h"
#include "system.h"
#include "timer.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;

	//ワールド作成
#ifdef _DEBUG
	m_scene = new TestScene();
#else
	m_scene = new TitleScene();
#endif // _DEBUG

	if (!m_scene->Initialize()) {
		return false;
	}

	//inputの初期化
	Input::Init();

	return true;
}

void Manager::Finalize() {
	//ワールドの終了
	m_scene->Finalize();
	delete m_scene;

	Input::Uninit();
}


void Manager::Update(double dt) {
	//Inputの更新
	Input::Update();

	//ワールドの更新
	m_scene->Update(dt);

}

void Manager::Draw() {
	//描画開始
	RENDERER.BeginDraw();

	//ワールドの描画
	m_scene->Draw();

	//描画終了
	RENDERER.EndDraw();

}

bool Manager::CleanUp() {
	if (m_isFinished) {
		return true;
	}

	//ワールドのクリーン
	m_scene->CleanUp();

	//シーン切り替え
	if (m_nextScene != nullptr) {
		m_scene->Finalize();
		delete m_scene;
		m_scene = m_nextScene;
		m_nextScene = nullptr;
		m_scene->Initialize();
		// 新しいシーンの初期化に時間がかかると次フレームの deltaTime が大きくなるため、
		// タイマーをリセットして次の Tick から安定した deltaTime を得る
		if (SYSTEM.GetTimer()) {
			SYSTEM.GetTimer()->Reset();
		}
	}

	return false;
}

void Manager::SetScene(Scene* scene) {
	m_nextScene = scene;
}


