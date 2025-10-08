#include "raceTimer.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include "sprite.h"
#include "renderer.h"
#include "texture.h"
#include "shaders.h"


bool RaceTimer::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"レースタイマーのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}
	//テクスチャの読み込み
	m_numberTexture = new Texture();
	if (!m_numberTexture->Load(L"Asset\\Texture\\dotNum.png")) {
		ErrorMessage(L"レースタイマーの数字テクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//コロンテクスチャの読み込み
	m_colonTexture = new Texture();
	if (!m_colonTexture->Load(L"Asset\\Texture\\colon.png")) {
		ErrorMessage(L"レースタイマーのコロンテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//アニメーションシェーダーの読み込み
	m_animationVertexShader = new VertexShader();
	m_animationVertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_animationPixelShader = new PixelShader();
	m_animationPixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	//位置、回転、拡大縮小の設定
	m_scale = { 60.0f, 112.0f, 1.0f };
	m_position = { SCREEN_WIDTH * 0.5f - m_scale.x * 3.0f + m_scale.x * 0.5f, 100.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	//レースマネージャーの取得
	m_raceManager = SYSTEM.GetManager()->GetScene()->GetGameObject<RaceManager>();

	return true;
}

void RaceTimer::Finalize() {
	//スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;
	m_sprite = nullptr;
	//テクスチャの解放
	delete m_numberTexture;
	m_numberTexture = nullptr;
	delete m_colonTexture;
	m_colonTexture = nullptr;
	//シェーダーの解放
	delete m_animationPixelShader;
	m_animationPixelShader = nullptr;
	delete m_animationVertexShader;
	m_animationVertexShader = nullptr;
}

void RaceTimer::Update(double deltaTime) {
	m_raceTime = m_raceManager->GetRaceTime();
}

void RaceTimer::Draw() const {
	//シェーダーの設定
	m_animationVertexShader->Set();
	m_animationPixelShader->Set();
	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//レースタイムを分秒ミリ秒に分解(例: 1分23秒45)
	int totalMilliseconds = static_cast<int>(m_raceTime * 1000.0f);
	int minutes = (totalMilliseconds / 60000) % 60;
	int seconds = (totalMilliseconds / 1000) % 60;
	int milliseconds = (totalMilliseconds / 10) % 100;

	//ポジション
	Vector3 pos = m_position;

	//テクスチャの設定
	m_numberTexture->Set(0);

	//アニメーション用プロパティ
	SHADER_PROPERTIES properties = {};
	properties.params1.x = 11.0f; //横フレーム数
	properties.params1.y = 1.0f;  //縦フレーム数

	//分
	properties.params1.z = static_cast<float>(minutes); //フレーム番号
	RENDERER.SetShaderProperties(properties);
	m_sprite->Draw(pos, m_rotation, m_scale);

	//コロン
	pos.x += m_scale.x; //右に移動
	m_colonTexture->Set(0);
	properties.params1.x = 1.0f; //フレーム数1
	properties.params1.z = 0.0f; //フレーム番号0
	RENDERER.SetShaderProperties(properties);
	m_sprite->Draw(pos, m_rotation, m_scale);

	//秒
	m_numberTexture->Set(0);
	properties.params1.x = 11.0f; //横フレーム数
	pos.x += m_scale.x; //右に移動
	for (int i = 0; i < 2; i++) {
		int digit = (seconds / static_cast<int>(std::pow(10, 1 - i))) % 10;
		properties.params1.z = static_cast<float>(digit); //フレーム番号
		RENDERER.SetShaderProperties(properties);
		m_sprite->Draw(pos, m_rotation, m_scale);
		pos.x += m_scale.x; //右に移動
	}

	//ドット
	properties.params1.z = 10.0f; //フレーム番号(ドット)
	RENDERER.SetShaderProperties(properties);
	m_sprite->Draw(pos, m_rotation, m_scale);
	pos.x += m_scale.x; //右に移動

	//ミリ秒
	for (int i = 0; i < 2; i++) {
		int digit = (milliseconds / static_cast<int>(std::pow(10, 1 - i))) % 10;
		properties.params1.z = static_cast<float>(digit); //フレーム番号
		RENDERER.SetShaderProperties(properties);
		m_sprite->Draw(pos, m_rotation, m_scale);
		pos.x += m_scale.x; //右に移動
	}
}
