#include "tmp2D.h"
#include "renderer.h"
#include "sprite.h"
#include "shaders.h"
#include "texture.h"

bool Temp2D::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"スプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\yukino.png")) {
		ErrorMessage(L"テクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_scale = { 450.0f, 600.0f, 1.0f };
	m_scale *= 0.2f; //縮小
	m_position = {m_scale.x * 0.5f, m_scale.y * 0.5f, 0.0f};

	return true;
}

void Temp2D::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;

	delete m_texture;
	m_texture = nullptr;

	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void Temp2D::Update(double deltaTime) {

}

void Temp2D::Draw() {
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
	//スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}

void Temp2D::CleanUp() {

}