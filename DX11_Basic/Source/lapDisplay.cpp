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
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");
	//レースマネージャー取得
	m_raceManager = m_scene->GetGameObject<RaceManager>();
	m_player = m_scene->GetGameObject<Player>();

	m_scale = { 30.f, 56.0f, 1.0f };
	m_position = { 20.0f + m_scale.x * 0.5f, SCREEN_HEIGHT - m_scale.y * 0.5f - 20.0f, 0.0f };

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
	if (m_pixelShader) {
		delete m_pixelShader;
	}
}

void LapDisplay::Update(double deltaTime) {
	m_lapCount = m_player->GetLapCount();

	m_maxLapCount = m_raceManager->GetLapCountToFinish();
}

void LapDisplay::Draw() const {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();
	//テクスチャの設定
	m_texture->Set(0);
	//マテリアルセット
	MATERIAL material = {};
	//金色(#FFD700)
	material.diffuse = XMFLOAT4(1.0f, 0.843f, 0.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//表示する数字の計算
	SHADER_PROPERTIES properties = { };
	properties.params1.z = 1.0f / 10.0f; //1フレームあたりの幅
	properties.params1.w = 1.0f / 2.0f; //1フレームあたりの高さ
	int displayNum = m_lapCount;
	if (displayNum > m_maxLapCount) {
		displayNum = m_maxLapCount;
	}
	properties.params1.x = static_cast<float>(displayNum % 10) * properties.params1.z; //フレームのX位置
	properties.params1.y = static_cast<float>(displayNum / 10) * properties.params1.w; //フレームのY位置
	RENDERER.SetShaderProperties(properties);
	//スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//スラッシュの描画
	displayNum = 12;
	properties.params1.x = static_cast<float>(displayNum % 10) * properties.params1.z; //フレームのX位置
	properties.params1.y = static_cast<float>(displayNum / 10) * properties.params1.w; //フレームのY位置
	RENDERER.SetShaderProperties(properties);

	Vector3 pos = m_position;
	pos.x += m_scale.x; //右にずらす
	m_sprite->Draw(pos, m_rotation, m_scale);

	//最大ラップ数の描画
	displayNum = m_maxLapCount;
	properties.params1.x = static_cast<float>(displayNum % 10) * properties.params1.z; //フレームのX位置
	properties.params1.y = static_cast<float>(displayNum / 10) * properties.params1.w; //フレームのY位置
	RENDERER.SetShaderProperties(properties);
	pos.x += m_scale.x; //さらに右にずらす
	m_sprite->Draw(pos, m_rotation, m_scale);
}
