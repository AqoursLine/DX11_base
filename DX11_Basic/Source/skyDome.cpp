#include "skyDome.h"
#include "renderer.h"
#include "model.h"
#include "texture.h"

bool SkyDome::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\skydome.fbx")) {
		ErrorMessage(L"スカイドームのモデル読み込みに失敗しました。", E_FAIL);
		return false;
	}

	m_scale = { 100.0f, 100.0f, 100.0f };

	return false;
}

void SkyDome::Finalize() {
	if (m_model) {
		delete m_model;
		m_model = nullptr;
	}

}

void SkyDome::Update(double deltaTime) {
}

void SkyDome::Draw() const {
	m_model->Draw(m_position, m_rotation, m_scale);
}
