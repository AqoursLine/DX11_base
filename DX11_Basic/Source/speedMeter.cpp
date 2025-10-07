#include "speedMeter.h"

#include "sprite.h"
#include "renderer.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "player.h"

bool SpeedMeter::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"スピードメーターのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}
	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\num.png")) {
		ErrorMessage(L"スピードメーターのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	m_scale = { 100.0f, 50.0f, 1.0f };
	m_position = { SCREEN_WIDTH - (m_scale.x * 3.0f), SCREEN_HEIGHT - (m_scale.y * 0.5f), 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	m_player = SYSTEM.GetManager()->GetScene()->GetGameObject<Player>();
	return true;
}

void SpeedMeter::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
	delete m_texture;
	m_texture = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void SpeedMeter::Update(double deltaTime) {
	if (m_player) {
		//速度を取得
		m_speed = m_player->GetSpeedKmh(); // km/h
	}
}

void SpeedMeter::Draw() const {
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
	for (int i = 0; i < 3; i++) {
		Vector3 pos = m_position;
		pos.x += m_scale.x * i; // 各桁の位置を調整
		
		// 100の位、10の位、1の位を計算
		int digit = static_cast<int>(m_speed) / static_cast<int>(std::pow(10, 2 - i)) % 10;
		// スプライトのUVオフセットを設定
		SHADER_PROPERTIES prop = {};
		prop.params1 = { 10.0f, 1.0f, static_cast<float>(digit), 0.0f };
		RENDERER.SetShaderProperties(prop);

		m_sprite->Draw(pos, m_rotation, m_scale);
	}


}
