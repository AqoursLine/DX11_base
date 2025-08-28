#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "scene.h"
#include "tmp2D.h"
#include "fieldObject.h"
#include "player.h"
#include "fpsCamera.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;

	//ワールド作成
	m_scene = new Scene();

	//ゆきのん初期化
	m_scene->AddGameObject(new Temp2D(), TYPE_2D);

	//フィールドオブジェクト
	m_scene->AddGameObject(new FieldObject(), TYPE_3D);

	//プレイヤー
	m_scene->AddGameObject(new Player(), TYPE_3D);

	//カメラの初期化
	m_scene->AddGameObject(new Camera(), TYPE_CAMERA);
//	m_scene->AddGameObject(new FpsCamera(), TYPE_CAMERA);

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

	return false;
}


