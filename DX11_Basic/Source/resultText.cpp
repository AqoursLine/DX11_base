#include "resultText.h"
#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool ResultText::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"結果テキストのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\result.png")) {
		ErrorMessage(L"タイトルテキストのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");
	m_backPixelShader = new PixelShader();
	m_backPixelShader->Load(L"Shader\\roundedRectPS.cso");

	m_position = { SCREEN_WIDTH * 0.5f, 200.0f, 0.0f };
	m_scale = { 800.0f, 200.0f, 1.0f };
	return true;
}

void ResultText::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
	delete m_texture;
	m_texture = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;

}

void ResultText::Update(double deltaTime) {
}

void ResultText::Draw() {
	// 背景の丸四角形描画
	// シェーダーの設定
	m_vertexShader->Set();
	m_backPixelShader->Set();

	// マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	material.textureEnable = false;
	RENDERER.SetMaterial(material);

	// プロパティ設定
	SHADER_PROPERTIES properties = {};
	properties.params1.x = 0.01f;	// 角丸半径
	properties.params1.y = 0.005f;	// 境界線のスムーズさ
	properties.params1.z = 1.0f;	// 矩形サイズ
	properties.params1.w = 1.0f;	// 矩形サイズ
	RENDERER.SetShaderProperties(properties);

	// スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//シェーダーの設定
	m_pixelShader->Set();
	//テクスチャの設定
	m_texture->Set(0);

	//マテリアルセット
	material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
