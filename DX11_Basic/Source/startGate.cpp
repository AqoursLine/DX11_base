#include "startGate.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool StartGate::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"スタートゲートのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}
	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\startGate.png")) {
		ErrorMessage(L"スタートゲートのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");
	//位置、回転、拡大縮小の設定
	m_scale = { 60.0f, 50.0f, 1.0f };
	m_position = { 0.0f, 10.0f, -30.0f };
	m_rotation = { 0.0f, -XM_PIDIV2, XM_PI };
	return true;
}

void StartGate::Finalize() {
	//スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;
	//テクスチャの解放
	delete m_texture;
	//シェーダーの解放
	delete m_vertexShader;
	delete m_pixelShader;
}

void StartGate::Update(double deltaTime) {
	//特に何もしない
}

void StartGate::Draw() const {
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
	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
