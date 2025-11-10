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

	//アニメーションシェーダーの読み込み
	m_animationVertexShader = new VertexShader();
	m_animationVertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_animationPixelShader = new PixelShader();
	m_animationPixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	//位置、回転、拡大縮小の設定
	m_scale = { 60.0f, 112.0f, 1.0f };
	m_position = { SCREEN_WIDTH * 0.5f + m_scale.x * 3.5f, 100.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	//レースマネージャーの取得
	m_raceManager = m_scene->GetGameObject<RaceManager>();

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
	material.diffuse = XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//ポジション
	Vector3 pos = m_position;

	//テクスチャの設定
	m_numberTexture->Set(0);

	//アニメーション用プロパティ
	SHADER_PROPERTIES properties = {};

	properties.params1.z = 1.0f / 10.0f; //1フレームの幅(10フレーム)
	properties.params1.w = 1.0f / 2.0f; //1フレームの高さ(2行)

	//ミリ秒を整数に変換して表示
	int timeInt = static_cast<int>(m_raceTime * 100); // 小数第2位まで表示

	//各桁の数字を取得
	int digits[7];
	digits[0] = (timeInt) % 10; // ミリ秒1の位
	digits[1] = (timeInt / 10) % 10; // ミリ秒10の位
	digits[2] = 10; // ドット
	digits[3] = (timeInt / 100) % 10; // 秒1の位
	digits[4] = (timeInt / 1000) % 6; // 秒10の位
	digits[5] = 11; // コロン
	digits[6] = (timeInt / 6000); // 分1の位
	

	//右から表示(1:00.00(分秒ミリ秒)形式)
	for (int i = 0; i < 7; i++) {
		//位置調整
		pos.x -= m_scale.x;

		//アニメーションプロパティ設定
		//左上のUV座標
		properties.params1.x = (digits[i] % 10) * properties.params1.z; //フレーム番号から左上のU座標を計算
		properties.params1.y = (digits[i] / 10) * properties.params1.w; //フレーム番号から左上のV座標を計算

		//シェーダープロパティ設定
		RENDERER.SetShaderProperties(properties);

		//スプライト描画
		m_sprite->Draw(pos, Vector3::ZERO, m_scale);
	}

}
