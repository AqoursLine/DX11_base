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
	if (!m_texture->Load(L"Asset\\Texture\\meterBackground.png")) {
		ErrorMessage(L"スピードメーターのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	m_numberTexture = new Texture();
	if (!m_numberTexture->Load(L"Asset\\Texture\\dotNum.png")) {
		ErrorMessage(L"スピードメーターの数字テクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	m_needleTexture = new Texture();
	if (!m_needleTexture->Load(L"Asset\\Texture\\needle.png")) {
		ErrorMessage(L"スピードメーターの針テクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//アニメーションシェーダーの読み込み
	m_animationVertexShader = new VertexShader();
	m_animationVertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_animationPixelShader = new PixelShader();
	m_animationPixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	//通常シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	//位置、回転、拡大縮小の設定
	m_scale = { 500.0f, 400.0f, 1.0f };
	m_position = { SCREEN_WIDTH - m_scale.x * 0.5f - 20.0f, SCREEN_HEIGHT - m_scale.y * 0.5f - 20.0f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	m_player = m_scene->GetGameObject<Player>();
	return true;
}

void SpeedMeter::Finalize() {
	//スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;

	//テクスチャの解放
	delete m_texture;
	m_texture = nullptr;
	delete m_numberTexture;
	m_numberTexture = nullptr;
	delete m_needleTexture;
	m_needleTexture = nullptr;

	//シェーダーの解放
	delete m_animationPixelShader;
	m_animationPixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
	delete m_animationVertexShader;
	m_animationVertexShader = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
}

void SpeedMeter::Update(double deltaTime) {
	if (m_player) {
		//速度を取得
		m_speed = m_player->GetSpeedKmh(); // km/h
	}
}

void SpeedMeter::Draw() {
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

	//背景描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//針描画
	m_needleTexture->Set(0);
	RENDERER.SetMaterial(material);

	//速度から針の角度を計算(0km/h = 0度, 100km/h = 180度)
	float angle = (m_speed / 100.0f) * 180.0f;
	if (angle > 180.0f) angle = 180.0f; //最大値制限
	Vector3 needleRot = { 0.0f, 0.0f, XMConvertToRadians(angle) };
	Vector3 needleScale = { m_scale.x * 0.8f, m_scale.y * 0.05f, 1.0f };
	Vector3 needlePos = m_position + Vector3(250.0f, 300.0f, 0.0f) - (m_scale * 0.5f);
	needlePos.z = 0.0f;
	m_sprite->Draw(needlePos, needleRot, needleScale);

	//数字描画(小数第2位まで)
	m_numberTexture->Set(0);
	//シェーダーの設定
	m_animationVertexShader->Set();
	m_animationPixelShader->Set();

	//速度を整数に変換して表示
	int speedInt = static_cast<int>(m_speed * 100); 

	Vector3 numScale = { m_scale.x * 0.06f, m_scale.y * 0.14f, 1.0f };
	//小数第2位の位置
	Vector3 numPos = m_position + Vector3(265.0f, 352.0f, 0.0f) - (m_scale * 0.5f);
	numPos.z = 0.0f;

	//マテリアル
	//#77FE7E
	material.diffuse = XMFLOAT4(0.467f, 0.996f, 0.494f, 1.0f);
	RENDERER.SetMaterial(material);

	//アニメーション用プロパティ
	SHADER_PROPERTIES properties = {};
	properties.params1.z = 1.0f / 10.0f; //1フレームの幅(10フレーム)
	properties.params1.w = 1.0f / 2.0f; //1フレームの高さ(2行)

	//小数点含めて6桁表示
	for (int i = 0; i < 6; i++) {
		int digit;
		if (i == 2) {
			//小数点
			digit = 10; //小数点のインデックス
		} else {
			digit = speedInt % 10; //一番右の桁を取得
			speedInt /= 10; //右にシフト
		}
		Vector3 digitPos = numPos - Vector3(numScale.x * i, 0.0f, 0.0f);

		//シェーダープロパティ設定
		properties.params1.x = (digit % 10) * properties.params1.z; //u座標
		properties.params1.y = (digit / 10) * properties.params1.w; //v座標
		RENDERER.SetShaderProperties(properties);

		//数字描画
		m_sprite->Draw(digitPos, Vector3::ZERO, numScale);
	}

}
