#include "resultTime.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "raceManager.h"

bool ResultTime::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"リザルトタイムのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\time.png")) {
		ErrorMessage(L"リザルトタイムのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	m_numberTexture = new Texture();
	if (!m_numberTexture->Load(L"Asset\\Texture\\dotNum.png")) {
		ErrorMessage(L"リザルトタイムの数字テクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	//位置、回転、拡大縮小の設定
	m_scale = { 250.0f, 100.0f, 1.0f };
	m_position = { m_scale.x * 0.5f + 20.0f, SCREEN_HEIGHT * 0.5f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	//レースタイムの取得
	m_resultTime = RaceManager::GetLaneTime(0);

	return true;
}

void ResultTime::Finalize() {
	//スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;
	//テクスチャの解放
	delete m_texture;
	delete m_numberTexture;
	//シェーダーの解放
	delete m_vertexShader;
	delete m_pixelShader;
}

void ResultTime::Update(double deltaTime) {
}

void ResultTime::Draw() const {
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

	//シェーダープロパティ設定
	SHADER_PROPERTIES properties = {};
	properties.params1.z = 1.0f; // 幅
	properties.params1.w = 1.0f; // 高さ
	RENDERER.SetShaderProperties(properties);

	//スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//数字の描画
	// スケール
	Vector3 numberScale = { 60.0f, 112.0f, 1.0f };
	 
	//ポジション
	Vector3 pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f };
	pos.x += numberScale.x * 3.5f; //右端の位置調整

	//テクスチャの設定
	m_numberTexture->Set(0);

	//アニメーション用プロパティ
	properties = {};
	properties.params1.z = 1.0f / 10.0f; //1フレームの幅(10フレーム)
	properties.params1.w = 1.0f / 2.0f; //1フレームの高さ(2行)

	//ミリ秒を整数に変換して表示
	int timeInt = static_cast<int>(m_resultTime * 100); // 小数第2位まで表示

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
		//アニメーションプロパティ設定
		//左上のUV座標
		properties.params1.x = (digits[i] % 10) * properties.params1.z; //フレーム番号から左上のU座標を計算
		properties.params1.y = (digits[i] / 10) * properties.params1.w; //フレーム番号から左上のV座標を計算

		//シェーダープロパティ設定
		RENDERER.SetShaderProperties(properties);

		//スプライト描画
		m_sprite->Draw(pos, Vector3::ZERO, numberScale);

		//位置調整
		pos.x -= numberScale.x;
	}

}
