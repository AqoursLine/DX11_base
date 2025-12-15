#include "testTransition.h"
#include "renderer.h"
#include "texture.h"
#include "sprite.h"
#include "shaders.h"

bool TestTransition::Initialize() {
	m_logoTexture = new Texture();
	m_logoTexture->Load(L"Asset\\Texture\\loading.png");
	m_fadeTexture = new Texture();
	m_fadeTexture->Load(L"Asset\\Texture\\white.jpg");

	m_spriteRenderer = new Sprite();
	m_spriteRenderer->Initialize();

	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");

	return true;
}

void TestTransition::Finalize() {
	delete m_logoTexture;
	delete m_fadeTexture;

	delete m_spriteRenderer;
	delete m_pixelShader;
	delete m_vertexShader;
}

void TestTransition::Draw() {
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	m_fadeTexture->Set(0);

	// マテリアル
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(0.3f, 0.3f, 1.0f, m_alpha);
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// フルスクリーンスプライト描画
	m_spriteRenderer->Draw({ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f });

	// ロゴスプライト描画
	m_logoTexture->Set(0);

	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_alpha);
	RENDERER.SetMaterial(material);

	m_spriteRenderer->Draw({ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f },
		{ 0.0f, 0.0f, m_logoRotate },
		{ 512.0f, 512.0f, 1.0f });
}

void TestTransition::UpdateInTransition(double deltaTime) {
	m_inTimer += static_cast<float>(deltaTime);

	m_alpha = 1.0f - (m_inTimer / m_inDuration);

	if (m_inTimer >= m_inDuration) {
		m_alpha = 0.0f;
		SetInTransitionFinished(true);
	}

	m_logoRotate += XMConvertToRadians(m_logoRotateSpeed) * static_cast<float>(deltaTime);
}

void TestTransition::UpdateTransition(double deltaTime) {
	m_logoRotate += XMConvertToRadians(m_logoRotateSpeed) * static_cast<float>(deltaTime);
}

void TestTransition::UpdateOutTransition(double deltaTime) {
	m_outTimer += static_cast<float>(deltaTime);
	m_alpha = m_outTimer / m_outDuration;

	if (m_outTimer >= m_outDuration) {
		m_alpha = 1.0f;
		SetOutTransitionFinished(true);
	}

	m_logoRotate += XMConvertToRadians(m_logoRotateSpeed) * static_cast<float>(deltaTime);
}
