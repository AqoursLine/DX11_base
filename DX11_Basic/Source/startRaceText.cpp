#include "startRaceText.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool StartRaceText::Initialize() {
	// スプライトの初期化
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}

	// テクスチャの読み込み
	m_background = new Texture();
	if (!m_background->Load(L"Asset/Texture/startRaceBack.png")) {
		return false;
	}

	m_textTexture = new Texture();
	if (!m_textTexture->Load(L"Asset/Texture/startRace.png")) {
		return false;
	}

	// シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader/unlitTextureVS.cso");

	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader/blockDissolvePS.cso");

	m_backgroundPS = new PixelShader();
	m_backgroundPS->Load(L"Shader/unlitColorPS.cso");

	// テキストと背景のスケール
	m_textScale = { 900.0f, 200.0f, 1.0f };
	m_backgroundScale = { 1920.0f, 300.0f, 1.0f };

	// テキストと背景の開始位置と終了位置
	m_textStartPos = { -m_textScale.x * 0.5f, 530.0f, 0.0f };
	m_textEndPos = { 960.0f, 530.0f, 0.0f };
	m_backgroundStartPos = { -m_backgroundScale.x * 0.5f, 540.0f, 0.0f };
	m_backgroundEndPos = { 960.0f, 540.0f, 0.0f };

    return true;
}

void StartRaceText::Finalize() {
	// リソースの解放
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}

	// テクスチャの解放
	if (m_background) {
		delete m_background;
		m_background = nullptr;
	}
	if (m_textTexture) {
		delete m_textTexture;
		m_textTexture = nullptr;
	}

	// シェーダーの解放
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}

}

void StartRaceText::Update(double deltaTime) {
	if (!m_isReady) {
		m_textPosition = m_textStartPos;
		m_backgroundPosition = m_backgroundStartPos;

		m_moveProgress = 0.0f;
		m_dissolveProgress = 0.0f;
		m_moveElapsed = 0.0f;

		return;
	}

	// 進行度更新(移動の後にディゾルブ)

	//　移動の進行度を更新
	m_moveProgress = static_cast<float>(m_moveElapsed / m_moveDuration);

	// 移動の経過時間を更新
	m_moveElapsed += static_cast<float>(deltaTime);

	// ディゾルブの進行度を更新
	m_dissolveProgress = static_cast<float>(m_moveElapsed / m_moveDuration);

	// 進行度を0.0fから1.0fの範囲にクランプ
	if (m_moveProgress > 1.0f) {
		m_moveProgress = 1.0f;
	}

	// テキストと背景の位置を線形補間で更新
	m_textPosition = m_textStartPos + (m_textEndPos - m_textStartPos) * m_moveProgress;
	m_backgroundPosition = m_backgroundStartPos + (m_backgroundEndPos - m_backgroundStartPos) * m_moveProgress;

}

void StartRaceText::Draw() {
	// レース開始の準備ができていない場合は描画しない
	if (!m_isReady) {
		return;
	}

	// 画面全体を暗くする背景
	m_vertexShader->Set();
	m_backgroundPS->Set();

	MATERIAL bgMaterial = {};
	bgMaterial.diffuse = { 0.0f, 0.0f, 0.0f, 0.5f }; // 半透明の黒
	bgMaterial.textureEnable = FALSE;
	RENDERER.SetMaterial(bgMaterial);

	// 描画
	m_sprite->Draw({ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f }, m_rotation, { SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f });

	// シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// マテリアルの設定
	MATERIAL material = {};
	material.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// シェーダープロパティの設定
	SHADER_PROPERTIES properties = {};
	properties.params1.x = 100.0f;
	properties.params1.y = 30.0f;
	properties.params1.z = m_moveElapsed / m_moveDuration;
	properties.params1.w = 1.0f;

	RENDERER.SetShaderProperties(properties);

	// 背景の描画
	m_background->Set();

	// 描画
	m_sprite->Draw(m_backgroundPosition, m_rotation, m_backgroundScale);

	// テキストの描画
	m_textTexture->Set();

	// 描画
	m_sprite->Draw(m_textPosition, m_rotation, m_textScale);
}
