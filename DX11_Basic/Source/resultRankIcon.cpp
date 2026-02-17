#include "resultRankIcon.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "raceManager.h"

bool ResultRankIcon::Initialize() {
	// スプライトの初期化
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"リザルトランクアイコンのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	// テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\triangle.png")) {
		ErrorMessage(L"リザルトランクアイコンのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	// シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	// 位置、回転、拡大縮小の設定
	m_scale = { 100.0f, 100.0f, 1.0f };
	m_position = { 700.0f, 400.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	auto resultData = RaceManager::GetResultData(); // 結果データ取得

	for (auto data : resultData) {
		m_colors.push_back(data.boatColor);
	}

	return true;
}

void ResultRankIcon::Finalize() {
	// スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;
	// テクスチャの解放
	delete m_texture;
	// シェーダーの解放
	delete m_vertexShader;
	delete m_pixelShader;
}

void ResultRankIcon::Update(double deltaTime) {
}

void ResultRankIcon::Draw() {
	// シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャの設定
	m_texture->Set(0);

	// ポジション
	Vector3 pos = m_position;

	// マテリアル
	MATERIAL material = {};
	material.textureEnable = TRUE;

	for (auto color : m_colors) {
		material.diffuse = XMFLOAT4(color.x, color.y, color.z, 1.0f);
		RENDERER.SetMaterial(material);

		// スプライトの描画
		m_sprite->Draw(pos, m_rotation, m_scale);

		pos.y += m_scale.y + 20.0f; // プレイヤーごとにY位置をずらす
	}

}
