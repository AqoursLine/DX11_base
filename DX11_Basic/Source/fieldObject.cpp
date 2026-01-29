#include "fieldObject.h"
#include "field.h"
#include "renderer.h"
#include "texture.h"
#include "shaders.h"

bool FieldObject::Initialize() {
	m_field = new Field();
	if (!m_field->Initialize()) {
		return false;
	}
	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\sura.jpg")) {
		ErrorMessage(L"フィールドのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_scale = {512.0, 1.0f, 512.0};
	m_position = { 0.0f, -10.0f, 0.0f };

	return true;
}

void FieldObject::Finalize() {
	if (m_field) {
		m_field->Finalize();
		delete m_field;
		m_field = nullptr;
	}
	delete m_texture;
	m_texture = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void FieldObject::Update(double deltaTime) {
}

void FieldObject::Draw() {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();
	//テクスチャの設定
	m_texture->Set(0);

	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//フィールドの描画
	m_field->Draw(m_position, m_rotation, m_scale);

}
