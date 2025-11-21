#include "countDownText.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"
#include "renderer.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include <algorithm>

bool RaceCountDownText::Initialize() {
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

	m_position = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.3f, 0.0f };
	m_scale = { 200.0f, 400.0f, 1.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	return true;
}

void RaceCountDownText::Finalize() {
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

void RaceCountDownText::Update(double deltaTime) {
	//レースマネージャーがない場合は処理しない
	if (!m_raceManager) {
		return;
	}

	//raceManagerのスタート前カウントダウン時間を取得
	float startDelay = m_raceManager->GetStartDelay();

	//表示時間を過ぎたら非表示にする
	if (startDelay <= 0.0f) {
		m_displayTime -= static_cast<float>(deltaTime);
		if (m_displayTime <= 0.0f) {
			SetVisible(false);
		}
		return;
	}

	//表示時間内なら表示
	m_time = startDelay + 0.5f;

}

void RaceCountDownText::Draw() {
	//レースマネージャーがない場合は処理しない
	if (!m_raceManager) {
		return;
	}

	//シェーダーセット
	m_vertexShader->Set();
	m_pixelShader->Set();
	//テクスチャセット
	m_texture->Set(0);
	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//定数バッファ更新
	SHADER_PROPERTIES props = {};

	props.params1.z = 1.0f / 10.0f; //1フレームの幅(10フレーム)
	props.params1.w = 1.0f / 2.0f; //1フレームの高さ(2行)

	//uv座標
	int frame = static_cast<int>(m_time);
	frame = std::clamp(frame, 0, 9); //0～9に制限

	props.params1.x = (frame % 10) * props.params1.z; //フレーム番号から左上のU座標を計算
	props.params1.y = (frame / 10) * props.params1.w; //フレーム番号から左上のV座標を計算

	RENDERER.SetShaderProperties(props);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
