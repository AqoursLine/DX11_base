#include "goalGate.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include "racingBoat.h"

bool GoalGate::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"ゴールゲートのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}
	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\goalTape.png")) {
		ErrorMessage(L"ゴールゲートのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	//周回ゲートのテクスチャの読み込み
	m_lapGateTexture = new Texture();
	if (!m_lapGateTexture->Load(L"Asset\\Texture\\lapTape.png")) {
		return false;
	}
	//番号テクスチャの読み込み
	m_lapNumberTexture = new Texture();
	if (!m_lapNumberTexture->Load(L"Asset\\Texture\\dotNum.png")) {
		return false;
	}

	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	m_lapGateVS = new VertexShader();
	m_lapGateVS->Load(L"Shader\\unlitTextureVS.cso");
	m_lapGatePS = new PixelShader();
	m_lapGatePS->Load(L"Shader\\addvancedSpriteAnimationPS.cso");

	//位置、回転、拡大縮小の設定
	m_scale = { 60.0f, 20.0f, 1.0f };
	m_position = { 0.0f, 10.0f, -30.0f };
	m_rotation = { 0.0f, -XM_PIDIV2, XM_PI };

	m_animationSpeed = 0.5f;

	//レースマネージャーの取得
	m_raceManager = SYSTEM.GetManager()->GetScene()->GetGameObject<RaceManager>();

	m_isPassed = false;

	SetVisible(false);
	return true;
}

void GoalGate::Finalize() {
	//スプライトの解放
	m_sprite->Finalize();
	delete m_sprite;
	//テクスチャの解放
	delete m_texture;
	delete m_lapGateTexture;
	delete m_lapNumberTexture;
	//シェーダーの解放
	delete m_vertexShader;
	delete m_pixelShader;
	delete m_lapGateVS;
	delete m_lapGatePS;
}

void GoalGate::Update(double deltaTime) {
	m_animationTime += static_cast<float>(deltaTime);

	auto racingBoats = SYSTEM.GetManager()->GetScene()->GetGameObjects<RacingBoat>();

	int minLapCount = INT_MAX;

	for (auto& boat : racingBoats) {
		if (!boat->IsLapUpdateReady()){
			continue;
		}


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

		if (isCollided) {
			boat->SetLapCount(boat->GetLapCount() + 1);

			boat->SetLapUpdateReady(false);

			if (boat->GetLapCount() > m_topLapCount) {
				m_topLapCount = boat->GetLapCount();
			}
			if (boat->GetLapCount() < minLapCount) {
				minLapCount = boat->GetLapCount();
			}

			if (boat->GetLapCount() > m_raceManager->GetLapCountToFinish()) {
				boat->SetPassedGoalGate(true);
				boat->FinishRace();
			}
		}
	}

	if (minLapCount != INT_MAX) {
		m_lastLapCount = minLapCount;
	}

	if (m_lastLapCount > 0) {
		SetVisible(true);
	} else {
		SetVisible(false);
	}
}

void GoalGate::Draw() const {

	//マテリアルセット
	MATERIAL material = {};
	material.textureEnable = true;

	//テクスチャの設定
	if (m_topLapCount >= m_raceManager->GetLapCountToFinish()) {
		//ゴールテープ
		//シェーダーの設定
		m_vertexShader->Set();
		m_pixelShader->Set();

		//テクスチャセット
		m_texture->Set(0);

		material.diffuse = XMFLOAT4(1.0f, 0.1f, 0.1f, 1.0f);
	} else {
		//周回ゲート
		//シェーダーの設定
		//m_lapGateVS->Set();
		//m_lapGatePS->Set();

		m_vertexShader->Set();
		m_pixelShader->Set();

		//テクスチャセット
		m_lapGateTexture->Set(0);
		m_lapNumberTexture->Set(1);

		material.diffuse = XMFLOAT4(0.85f, 1.0f, 0.0f, 1.0f);
	}

	RENDERER.SetMaterial(material);

	//シェーダープロパティ
	SHADER_PROPERTIES properties = {};
	//params1 ゲートのUVアニメーション用
	properties.params1.z = 2.0f; //幅
	properties.params1.w = 1.0f; //高さ
	properties.params1.x = m_animationTime * m_animationSpeed; //アニメーション速度

	//params2 番号テクスチャ用
	properties.params2.z = 1.0f / 10.0f; //1フレームの幅(10フレーム)
	properties.params2.w = 1.0f / 2.0f; //1フレームの高さ
	properties.params2.x = static_cast<float>(m_lastLapCount % 10) * properties.params2.z; //Xオフセット
	properties.params2.y = static_cast<float>(m_lastLapCount / 10) * properties.params2.w; //Yオフセット

	//params3 ゲート上の番号位置用
	properties.params3.x = 0.1f; //uv上の最小X位置
	properties.params3.y = 0.22f; //uv上の最小Y位置
	properties.params3.z = 0.3f; //uv上の最大X位置
	properties.params3.w = 0.78f; //uv上の最大Y位置


	RENDERER.SetShaderProperties(properties);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
