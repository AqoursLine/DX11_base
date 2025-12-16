#include "titleBackgroundMove.h"
#include "video_texture.h"

#include "renderer.h"
#include "shaders.h"
#include "sprite.h"

bool TitleBackgroundMove::Initialize() {
	m_videoTexture = new VideoTexture();
	if (m_videoTexture) {
		if (!m_videoTexture->create("Asset\\Movie\\demo.mp4")) {
			return false;
		}
	}

	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\videoPS.cso");

	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}

	m_scale = { SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f };
	m_position = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	return true;
}

void TitleBackgroundMove::Finalize() {
	if (m_videoTexture) {
		m_videoTexture->destroy();
		delete m_videoTexture;
		m_videoTexture = nullptr;
	}
	m_sprite->Finalize();
	delete m_sprite;
	m_sprite = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void TitleBackgroundMove::Update(double deltaTime) {
	if (m_videoTexture) {
		m_videoTexture->update(static_cast<float>(deltaTime));
	}
}

void TitleBackgroundMove::Draw() {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	//テクスチャの設定
	if (m_videoTexture) {
		auto texture = m_videoTexture->getTexture();
		if (texture) {
			texture->activate(0);
		}
	}

	// マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

}
