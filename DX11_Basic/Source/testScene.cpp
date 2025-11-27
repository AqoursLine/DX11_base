#include "main.h"
#include "testScene.h"

#include "input.h"

#include "system.h"
#include "manager.h"
#include "titleScene.h"

#include "renderer.h"

#include "test.h"
#include "fpsCamera.h"
#include "skyDome.h"
#include "testParticle.h"
#include "splashParticle.h"

#include "gameDirectionalLight.h"

#include "imguiSystem.h"
#include "testField.h"
#include "testSprite.h"

bool TestScene::Initialize() {
	//テストオブジェクト追加
	AddGameObject(new TestObject(), TYPE_OPAQUE);

	//カメラ追加
	AddGameObject(new FpsCamera(), TYPE_CAMERA)->SetPosition({ 0.0f, 5.0f, -20.0f });

	//スカイドーム追加
//	AddGameObject(new SkyDome(), TYPE_OPAQUE);

	//平行光源追加
	auto light = new GameDirectionalLight();
	Vector4 dir = { 0.0f, -1.0f, 1.0f, 0.0f };
	light->SetDirection(dir)
	->SetEnabled(true)
	->SetShadowCaster(true)
	->SetPosition({ 0.0f, 20.0f, 0.0f });
	AddGameObject(light, TYPE_LIGHT);

	//平行光源追加
	//light = new GameDirectionalLight();
	//dir = { 1.0f, 0.0f, -1.0f, 0.0f };
	//light->SetDirection(dir)
	//->SetEnabled(true);
	//AddGameObject(light, TYPE_LIGHT);

//	AddGameObject(new TestParticle, TYPE_TRANSPARENT);

//	AddGameObject(new SplashParticle(), TYPE_TRANSPARENT);

//	AddGameObject(new TestField(), TYPE_OPAQUE);

	AddGameObject(new TestSprite(), TYPE_BEFORE_PROCESS_UI);

	return true;
}

void TestScene::Finalize() {
}

void TestScene::Update(double deltaTime) {
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene());
	}
}

void TestScene::Draw() {
	RENDERER.ClearShadowMap(0);
}
