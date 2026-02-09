#include "lapDisplay.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"
#include "player.h"

bool LapDisplay::Initialize() {
	//スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	//テクスチャロード
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\dotNum.png")) {
		return false;
	}
	//シェーダーロード
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_backVertexShader = new VertexShader();
	m_backVertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");
	m_backPixelShader = new PixelShader();
	m_backPixelShader->Load(L"Shader\\roundedRectPS.cso");

	//レースマネージャー取得
	m_raceManager = m_scene->GetGameObject<RaceManager>();
	m_player = m_scene->GetGameObject<Player>();

	if (!m_player) {
		SetActive(false);
	}

	//位置、回転、拡大縮小の設定
	m_numberScale = { 30.f, 56.0f, 1.0f };
	m_scale = { 100.0f, 70.0f, 1.0f };
	m_position = { 150.0f + m_scale.x * 0.5f, SCREEN_HEIGHT - m_scale.y * 0.5f - 20.0f, 0.0f };

	return true;
}

void LapDisplay::Finalize() {
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
	}
	if (m_texture) {
		delete m_texture;
	}
	if (m_vertexShader) {
		delete m_vertexShader;
	}
	if (m_backVertexShader) {
		delete m_backVertexShader;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
	}
	if (m_backPixelShader) {
		delete m_backPixelShader;
	}
}

void LapDisplay::Update(double deltaTime) {
	m_lapCount = m_player->GetLapCount();

	m_maxLapCount = m_raceManager->GetLapCountToFinish();
}

void LapDisplay::Draw() {
	// バックグラウンドの描画
	//シェーダーの設定
	m_backVertexShader->Set();
	m_backPixelShader->Set();

	//マテリアルセット
	MATERIAL backMaterial = {};

	//半透明黒
	backMaterial.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	backMaterial.textureEnable = false;
	RENDERER.SetMaterial(backMaterial);

	// シェーダープロパティセット
	SHADER_PROPERTIES properties = {};
	properties.params1.x = 0.1f;	// 角丸半径
	properties.params1.y = 0.005f;	// 境界線のスムーズさ
	properties.params1.z = 1.0f;	// 矩形サイズ
	properties.params1.w = 1.0f;	// 矩形サイズ
	RENDERER.SetShaderProperties(properties);

	//スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//前景の描画
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();
	//テクスチャの設定
	m_texture->Set(0);
	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 0.843f, 0.0f, 1.0f);	//金色(#FFD700)
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	// 描画位置オフセット
	Vector3 offset = { m_numberScale.x, 0.0f, 0.0f };

	//表示する数字の計算
	properties = {};
	properties.params1.z = 1.0f / 10.0f; //1フレームあたりの幅
	properties.params1.w = 1.0f / 2.0f; //1フレームあたりの高さ
	int displayNum = m_lapCount;
	if (displayNum > m_maxLapCount) {
		displayNum = m_maxLapCount;
	}

	//現在のラップ数の描画
	properties.params1.x = static_cast<float>(displayNum % 10) * properties.params1.z; //フレームのX位置
	properties.params1.y = static_cast<float>(displayNum / 10) * properties.params1.w; //フレームのY位置
	RENDERER.SetShaderProperties(properties);
	m_sprite->Draw(m_position - offset, m_rotation, m_numberScale);

	//スラッシュの描画
	displayNum = 12;
	properties.params1.x = static_cast<float>(displayNum % 10) * properties.params1.z; //フレームのX位置
	properties.params1.y = static_cast<float>(displayNum / 10) * properties.params1.w; //フレームのY位置
	RENDERER.SetShaderProperties(properties);

	m_sprite->Draw(m_position, m_rotation, m_numberScale);

	//最大ラップ数の描画
	displayNum = m_maxLapCount;
	properties.params1.x = static_cast<float>(displayNum % 10) * properties.params1.z; //フレームのX位置
	properties.params1.y = static_cast<float>(displayNum / 10) * properties.params1.w; //フレームのY位置
	RENDERER.SetShaderProperties(properties);
	m_sprite->Draw(m_position + offset, m_rotation, m_numberScale);
}
