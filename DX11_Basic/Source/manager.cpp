#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "titleScene.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;

	//ワールド作成
	m_scene = new TitleScene();
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
	}

	return false;
}

void Manager::SetScene(Scene* scene) {
	m_nextScene = scene;
}


