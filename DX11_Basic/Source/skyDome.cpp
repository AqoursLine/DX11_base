#include "skyDome.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "camera.h"

bool SkyDome::Initialize() {

	m_model = new ModelRenderer();
	if (!m_model->Load("Asset\\Model\\skydome.fbx")) {
		ErrorMessage(L"モデルの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーロード
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_scale = { 100000.0f, 100000.0f, 100000.0f };

	return true;
}

void SkyDome::Finalize() {
	if (m_model) {
		delete m_model;
		m_model = nullptr;
	}
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}

}

void SkyDome::Update(double deltaTime) {
	auto camera = SYSTEM.GetManager()->GetScene()->GetGameObject<Camera>();

	if (camera) {
		m_position = camera->GetPosition();
	}
}

void SkyDome::Draw() const {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();
	//描画
	m_model->Draw(m_position, m_rotation, m_scale);
}
