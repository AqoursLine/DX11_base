#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "testScene.h"
#include "titleScene.h"
#include "system.h"
#include "timer.h"
#include "imguiSystem.h"

Manager::Manager() {
	m_imguiSystem = new ImguiSystem();
}

Manager::~Manager() {
	if (m_imguiSystem) {
		delete m_imguiSystem;
		m_imguiSystem = nullptr;
	}
}

bool Manager::Initialize() {
	m_isFinished = false;

	//ImGuiシステム初期化
	if (!m_imguiSystem->Initialize(GetHwnd(), RENDERER.GetDevice(), RENDERER.GetDeviceContext())) {
		return false;
	}

	//ワールド作成
#ifdef _DEBUG
	m_scene = new TestScene();
#else
	m_scene = new TitleScene();
#endif // _DEBUG

	if (!m_scene->InitializeBase()) {
		return false;
	}

	//inputの初期化
	Input::Init();

	return true;
}

void Manager::Finalize() {
	//ワールドの終了
	m_scene->FinalizeBase();
	delete m_scene;

	Input::Uninit();
}


void Manager::Update(double dt) {
	//Inputの更新
	Input::Update();

	//ワールドの更新
	m_scene->UpdateBase(dt);

}

void Manager::Draw() {
	//描画開始
	RENDERER.BeginDraw();

	//ImGui描画開始
	m_imguiSystem->Begin();

	//ワールドの描画
	m_scene->DrawBase();

	// deltaTime表示
#ifdef _DEBUG
	ImGui::SetNextWindowSize(ImVec2(240, 50));
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::Begin("Debug Info");
	ImGui::Text("Delta Time: %.6f seconds", SYSTEM.GetTimer()->GetDeltaTime());
	ImGui::End();
#endif // _DEBUG

	//ImGui描画終了
	m_imguiSystem->End();

	//描画終了
	RENDERER.EndDraw();

}

bool Manager::CleanUp() {
	if (m_isFinished) {
		return true;
	}

	//ワールドのクリーン
	m_scene->CleanUpBase();

	//シーン切り替え
	if (m_nextScene != nullptr) {
		if (!m_nextScene->IsInitializedBase()) {
			m_nextScene->InitializeBase();
		} else if (m_nextScene->IsInitialized()) {
			m_scene->FinalizeBase();
			delete m_scene;

			m_scene = m_nextScene;
			m_nextScene = nullptr;
		}
	}

	return false;
}

void Manager::SetScene(Scene* scene) {
	m_nextScene = scene;
}


