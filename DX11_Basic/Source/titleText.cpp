#include "titleText.h"
#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool TitleText::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"タイトルテキストのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\title.jpg")) {
		ErrorMessage(L"タイトルテキストのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_position = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f };
	m_scale = { 800.0f, 200.0f, 1.0f };

#ifdef _DEBUG
	//テスト用にこのスレッドをしばらく待機
	Sleep(5000);

#endif // _DEBUG

	return true;
}

void TitleText::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
	delete m_texture;
	m_texture = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;

}

void TitleText::Update(double deltaTime) {
}

void TitleText::Draw() {
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
