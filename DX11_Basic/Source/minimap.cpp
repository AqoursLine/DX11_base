#include "main.h"
#include "minimap.h"
#include "sprite.h"
#include "renderer.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "racingBoat.h"


bool MiniMap::Initialize() {
	//テクスチャの読み込み
	m_minimapTexture = new Texture();
	if (!m_minimapTexture->Load(L"Asset\\Texture\\raceCourse.png")) {
		ErrorMessage(L"ミニマップのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	m_playerIconTexture = new Texture();
	if (!m_playerIconTexture->Load(L"Asset\\Texture\\playerIcon.png")) {
		ErrorMessage(L"プレイヤーアイコンのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	m_buoyIconTexture = new Texture();
	if (!m_buoyIconTexture->Load(L"Asset\\Texture\\buoyIcon.png")) {
		ErrorMessage(L"ブイアイコンのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーの読み込み
	m_minimapVS = new VertexShader();
	m_minimapVS->Load(L"Shader\\unlitTextureVS.cso");
	m_minimapPS = new PixelShader();
	m_minimapPS->Load(L"Shader\\unlitTexturePS.cso");

	//スプライト
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}

	//位置とサイズの設定
	m_scale = { 500.0f , 500.0f, 1.0f };
	m_position = { SCREEN_WIDTH - (m_scale.x * 0.5f), SCREEN_HEIGHT * 0.5f, 0.0f };
	m_rotation = { 0.0f, 0.0f, 0.0f };

	//レースボートの取得
	m_racingBoats = SYSTEM.GetManager()->GetScene()->GetGameObjects<RacingBoat>();

	//マップ変換率の設定
	m_mapConversion = 0.9f; //ワールド座標からミニマップ座標への変換率

	return true;
}

void MiniMap::Finalize() {
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}
	if (m_playerIconTexture) {
		delete m_playerIconTexture;
		m_playerIconTexture = nullptr;
	}
	if (m_buoyIconTexture) {
		delete m_buoyIconTexture;
		m_buoyIconTexture = nullptr;
	}
	if (m_minimapTexture) {
		delete m_minimapTexture;
		m_minimapTexture = nullptr;
	}
	if (m_minimapPS) {
		delete m_minimapPS;
		m_minimapPS = nullptr;
	}
	if (m_minimapVS) {
		delete m_minimapVS;
		m_minimapVS = nullptr;
	}
}

void MiniMap::Update(double deltaTime) {
}

void MiniMap::Draw() const {
	//シェーダーの設定
	m_minimapVS->Set();
	m_minimapPS->Set();
	//テクスチャの設定
	m_minimapTexture->Set(0);
	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//テクスチャの設定
	m_playerIconTexture->Set(0);

	//マテリアル設定
	material.diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.3f);

	//プレイヤーアイコン描画
	for (const auto& boat : m_racingBoats) {
		Vector3 pos = boat->GetPosition();
		Vector3 rot = boat->GetRotation();

		//ミニマップ上の位置に変換
		Vector3 iconPos = {};
		iconPos.x = m_position.x + (pos.x * m_mapConversion);
		iconPos.y = m_position.y - (pos.z * m_mapConversion); //Z軸が奥行きなので反転
		iconPos.z = 0.0f;

		//ミニマップ上の回転に変換
		Vector3 iconRot = {};
		iconRot.z = rot.y; //Y軸回転をZ軸回転に変換

		//マテリアルセット
		RENDERER.SetMaterial(material);

		m_sprite->Draw(iconPos, iconRot, m_iconScale);
	}

	//ブイアイコン描画
	m_buoyIconTexture->Set(0);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	RENDERER.SetMaterial(material);

	Vector3 buoyPos = {};
	buoyPos.x = m_position.x + (150.0f * m_mapConversion);
	buoyPos.y = m_position.y - (0 * m_mapConversion);
	buoyPos.z = 0.0f;

	Vector3 scale = m_iconScale * 2.0f;

	m_sprite->Draw(buoyPos, {}, scale);
	buoyPos.x = m_position.x + (-150.0f * m_mapConversion);
	m_sprite->Draw(buoyPos, {}, scale);
}
