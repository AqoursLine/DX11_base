#include "main.h"
#include "manager.h"
#include "DX11/renderer.h"
#include "GameObject/camera.h"
#include "System/input.h"
#include "GameObject/world.h"
#include "GameObject/tmp2D.h"
#include "GameObject/fieldObject.h"
#include "GameObject/player.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;

	//ワールド作成
	m_world = new World();

	//ゆきのん初期化
	m_world->AddGameObject(new Temp2D());

	//フィールドオブジェクト
	m_world->AddGameObject(new FieldObject());

	//プレイヤー
	m_world->AddGameObject(new Player());

	//カメラの初期化
	m_world->AddGameObject(new Camera());

	//inputの初期化
	Input::Init();

	return true;
}

void Manager::Finalize() {
	//ワールドの終了
	m_world->Finalize();
	delete m_world;

	Input::Uninit();
}


void Manager::Update(double dt) {
	//Inputの更新
	Input::Update();

	//ワールドの更新
	m_world->Update(dt);

}

void Manager::Draw() const {
	//描画開始
	RENDERER.BeginDraw();

	//ワールドの描画
	m_world->Draw();

	//描画終了
	RENDERER.EndDraw();

}

bool Manager::CleanUp() {
	if (m_isFinished) {
		return true;
	}

	//ワールドのクリーン
	m_world->CleanUp();

	return false;
}


