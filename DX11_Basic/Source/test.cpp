#include "test.h"

#include "system.h"
#include "manager.h"
#include "scene.h"

#include "input.h"

#include "renderer.h"
#include "modelRenderer.h"
#include "box.h"

#include "shaders.h"

#include "imguiSystem.h"

bool TestObject::Initialize() {
	m_modelRenderer = new ModelRenderer();
	if (!m_modelRenderer->Load("Asset\\Model\\torus3.fbx")) {
		ErrorMessage(L"モデルの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	m_box = new Box();
	if (!m_box->Initialize()) {
		ErrorMessage(L"ボックスの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーロード
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\pixelLightingVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\lightTestPS.cso");

	//位置、回転、拡大縮小の設定
	m_position = { 0.0f, 2.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.0f, 1.0f, 1.0f };
	 return true;
}

void TestObject::Finalize() {
	if (m_modelRenderer) {
		delete m_modelRenderer;
		m_modelRenderer = nullptr;
	}
	if (m_box) {
		m_box->Finalize();
		delete m_box;
		m_box = nullptr;
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

void TestObject::Update(double deltaTime) {

}

void TestObject::DrawShadow() {
	if (m_modelRenderer) {
		m_modelRenderer->Draw(m_position, m_rotation, m_scale);
	}

	if (m_box) {
		m_box->Draw(m_position + Vector3(3.0f, 0.0f, 0.0f), m_rotation, m_scale);
	}


}

void TestObject::Draw() {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	if (m_modelRenderer) {
		m_modelRenderer->Draw(m_position, m_rotation, m_scale);
	}

	if (m_box) {
		m_box->Draw(m_position + Vector3(3.0f, 0.0f, 0.0f), m_rotation, m_scale);
	}

#ifdef _DEBUG
	// ImGuiウィンドウの表示
	ImGui::Begin("Test Object");
	ImGui::Text("Position");
	ImGui::SliderFloat("X", &m_position.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Y", &m_position.y, -10.0f, 10.0f);
	ImGui::SliderFloat("Z", &m_position.z, -10.0f, 10.0f);
	ImGui::Text("Rotation");
	ImGui::SliderFloat("Rot X", &m_rotation.x, -XM_PI, XM_PI);
	ImGui::SliderFloat("Rot Y", &m_rotation.y, -XM_PI, XM_PI);
	ImGui::SliderFloat("Rot Z", &m_rotation.z, -XM_PI, XM_PI);
	ImGui::Text("Scale");
	ImGui::SliderFloat("Scale X", &m_scale.x, 0.1f, 5.0f);
	ImGui::SliderFloat("Scale Y", &m_scale.y, 0.1f, 5.0f);
	ImGui::SliderFloat("Scale Z", &m_scale.z, 0.1f, 5.0f);
	ImGui::End();

#endif // _DEBUG

}
