#include "main.h"
#include "testScene.h"

#include "input.h"

#include "system.h"
#include "manager.h"
#include "titleScene.h"

#include "test.h"
#include "fpsCamera.h"
#include "skyDome.h"
#include "particleSystem.h"
#include "texture.h"

#include "gameDirectionalLight.h"

bool TestScene::Initialize() {
	//テストオブジェクト追加
	AddGameObject(new TestObject(), TYPE_OPAQUE);

	//カメラ追加
	AddGameObject(new FpsCamera(), TYPE_CAMERA)->SetPosition({ 0.0f, 5.0f, -20.0f });

	//スカイドーム追加
	AddGameObject(new SkyDome(), TYPE_OPAQUE);

	//平行光源追加
	auto light = new GameDirectionalLight();
	Vector4 dir = { 0.0f, -1.0f, 0.0f, 0.0f };
	dir.Normalize();
	light->SetDirection(dir);
	light->SetEnabled(true);
	AddGameObject(light, TYPE_LIGHT);

	//平行光源追加
	light = new GameDirectionalLight();
	dir = { 1.0f, 0.0f, -1.0f, 0.0f };
	dir.Normalize();
	light->SetDirection(dir);
	light->SetEnabled(true);
	AddGameObject(light, TYPE_LIGHT);

	m_circleTex = new Texture();
	m_circleTex->Load(L"Asset\\Texture\\circle.png");

	auto ps = new ParticleSystem();
	ps->SetTexture(m_circleTex->GetSRV());

	EmitterSettings settings;
	settings.startColor = { 1.0f, 0.2f, 0.2f, 1.0f };
	settings.endColor = { 1.0f, 1.0f, 0.2f, 1.0f };
	settings.startSize = 1.0f;
	settings.endSize = 0.1f;
	settings.lifeTime = 2.0f;
	settings.position = { 0.0f, 0.0f, 0.0f };
	settings.velocity = { 0.0f, 5.0f, 0.0f };
	settings.velocityVariation = { 1.0f, 1.0f, 1.0f };
	settings.gravity = 3.0f;

	ps->SetEmitterSettings(settings);
	AddGameObject(ps, TYPE_TRANSPARENT);

	return true;
}

void TestScene::Finalize() {
	delete m_circleTex;
}

void TestScene::Update(double deltaTime) {
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene());
	}
}
