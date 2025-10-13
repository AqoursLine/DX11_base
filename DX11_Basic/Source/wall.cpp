#include "wall.h"
#include "box.h"
#include "shaders.h"

#include "renderer.h"

bool Wall::Initialize() {
	m_model = new Box();
	if (!m_model->Initialize()) {
		return false;
	}
	//シェーダーロード
	m_vs = new VertexShader();
	m_vs->Load(L"Shader\\unlitTextureVS.cso");
	m_ps = new PixelShader();
	m_ps->Load(L"Shader\\gridPS.cso");
	return true;
}

void Wall::Finalize() {
	if (m_model) {
		m_model->Finalize();
		delete m_model;
	}
	if (m_vs) {
		delete m_vs;
		m_vs = nullptr;
	}
	if (m_ps) {
		delete m_ps;
		m_ps = nullptr;
	}
}

void Wall::Update(double deltaTime) {
}

void Wall::Draw() const {
	if (m_model) {
		//シェーダーの設定
		m_vs->Set();
		m_ps->Set();

		//モデルの描画
		m_model->Draw(m_position, m_rotation, m_scale);
	}
}
