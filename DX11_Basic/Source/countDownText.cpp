#include "countDownText.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"
#include "renderer.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

bool RaceCountDownText::Initialize() {
	//スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	//テクスチャロード
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\num.png")) {
		return false;
	}
	//シェーダーロード
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");

	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	//レースマネージャー取得
	m_raceManager = SYSTEM.GetManager()->GetScene()->GetGameObject<RaceManager>();

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
	m_time = startDelay + 1.0f;

}

void RaceCountDownText::Draw() const {
	//レースマネージャーがない場合は処理しない
	if (!m_raceManager) {
		return;
	}

	//定数バッファ更新
	SHADER_PROPERTIES props = {};
	props.params1 = Vector4(10.0f, 1.0f, m_time, 0.0f);
	RENDERER.SetShaderProperties(props);

	//シェーダーセット
	m_vertexShader->Set();
	m_pixelShader->Set();
	//テクスチャセット
	m_texture->Set(0);
	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
