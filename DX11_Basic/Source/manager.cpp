#include "main.h"
#include "manager.h"
#include "DX11/renderer.h"
#include "sprite.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;

	m_sprite = new Sprite();
	if (!m_sprite->Initialize(L"Asset\\Texture\\yukino.png")) {
		ErrorMessage(L"スプライトの初期化に失敗しました", E_FAIL);
		return false;
	}

	return true;
}

void Manager::Finalize() {
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}
}


bool Manager::Update(double dt) {

	if (m_isFinished) {
		return true;
	}

	return false;
}

void Manager::Draw() const {
	//描画開始
	RENDERER.BeginDraw();

	//描画処理
	m_sprite->Draw(Vector3(500.0f, 500.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(200.0f, 200.0f, 1.0f));

	//描画終了
	RENDERER.EndDraw();

}

void Manager::CleanUp() {
}


