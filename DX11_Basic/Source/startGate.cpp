#include "startGate.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include "racingBoat.h"

bool StartGate::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"スタートゲートのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}
	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\startTape.png")) {
		ErrorMessage(L"スタートゲートのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");
	//位置、回転、拡大縮小の設定
	m_scale = { 60.0f, 20.0f, 1.0f };
	m_position = { 0.0f, 10.0f, -30.0f };
	m_rotation = { 0.0f, -XM_PIDIV2, XM_PI };

	m_animationSpeed = 0.5f;

	//レースマネージャーの取得
	m_raceManager = m_scene->GetGameObject<RaceManager>();

	m_isPassed = false;
	m_passCheckTime = 0.0f;
	return true;
}

void StartGate::Finalize() {
	//スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;
	//テクスチャの解放
	delete m_texture;
	//シェーダーの解放
	delete m_vertexShader;
	delete m_pixelShader;
}

void StartGate::Update(double deltaTime) {
	m_animationTime += static_cast<float>(deltaTime);

	//スタートゲート通過チェック
	if (m_isPassed) {
		m_passCheckTime += static_cast<float>(deltaTime);

		if (m_passCheckTime >= 1.0f) {
			SetActive(false);
			return;
		}
	}

	auto racingBoats = m_scene->GetGameObjects<RacingBoat>();

	for (auto& boat : racingBoats) {
		if (!boat->IsPassedStartGate()) {
			//まだ通過していないボートがいる場合
			Vector3 boatPos = boat->GetPosition();

			bool isCollided = false;

			if (boatPos.z < m_position.z - m_scale.z * 0.5f && boatPos.z > m_position.z + m_scale.z * 0.5f) {
				continue;
			}

			float boatHalfLength = boat->GetLength() * 0.5f;

			if (boatPos.x + boatHalfLength > m_position.x - m_scale.x * 0.05f && boatPos.x + boatHalfLength < m_position.x + m_scale.x * 0.05f) {
				isCollided = true;
			}

			Vector3 boatVel = boat->GetVelocity();
			boatVel.y = 0.0f; //水平成分のみ

			isCollided = isCollided && (boatVel.Dot(Vector4::FromAxisAngle(Vector3::UP, m_rotation.y).RotateVector(Vector3::FORWARD)) < 0.0f);

			if (isCollided && m_passCheckTime < 1.0f) {
				boat->SetPassedStartGate(true);
				m_isPassed = true;
				m_raceManager->SetRaceStarted(true);
			}

		}
	}

	//全ボートが通過した場合、ゲートを非アクティブにする
	bool allPassed = true;
	for (auto& boat : racingBoats) {
		if (!boat->IsPassedStartGate()) {
			allPassed = false;
			break;
		}
	}
	if (allPassed) {
		SetActive(false);
	}
}

void StartGate::Draw() {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();
	//テクスチャの設定
	m_texture->Set(0);
	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(0.0f, 1.0f, 0.5f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//シェーダーに時間を渡す
	SHADER_PROPERTIES properties = {};
	properties.params1.z = 2.0f; //幅
	properties.params1.w = 1.0f; //高さ
	properties.params1.x = m_animationTime * m_animationSpeed; //アニメーション速度
	RENDERER.SetShaderProperties(properties);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
