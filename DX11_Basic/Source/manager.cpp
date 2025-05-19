#include "main.h"
#include "manager.h"
#include "DX11/renderer.h"
#include "sprite.h"
#include "camera.h"
#include "field.h"
#include "System/input.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;

	//スプライトの初期化
	m_sprite = new Sprite();
	if (!m_sprite->Initialize(L"Asset\\Texture\\yukino.png")) {
		ErrorMessage(L"スプライトの初期化に失敗しました", E_FAIL);
		return false;
	}

	//カメラの初期化
	m_camera = new Camera();
	m_camera->Initialize();

	//フィールドの初期化
	m_field = new Field();
	if (!m_field->Initialize(L"Asset\\Texture\\大崎甜花_ゲーミング.png")) {
		ErrorMessage(L"スプライトの初期化に失敗しました", E_FAIL);
		return false;
	}

	//inputの初期化
	Input::Init();

	return true;
}

void Manager::Finalize() {
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}

	if (m_camera) {
		m_camera->Finalize();
		delete m_camera;
		m_camera = nullptr;
	}

	if (m_field) {
		m_field->Finalize();
		delete m_field;
		m_field = nullptr;
	}

	Input::Uninit();
}


void Manager::Update(double dt) {
	//Inputの更新
	Input::Update();

	//カメラの更新
	m_camera->Update(dt);
}

void Manager::Draw() const {
	//描画開始
	RENDERER.BeginDraw();

	//カメラの描画
	m_camera->Draw();

	//フィールドの描画
	m_field->Draw(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(16.0f, 1.0f, 9.0f));

	//描画処理
	m_sprite->Draw(Vector3(500.0f, 500.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));

	//描画終了
	RENDERER.EndDraw();

}

bool Manager::CleanUp() {
	if (m_isFinished) {
		return true;
	}

	return false;
}


