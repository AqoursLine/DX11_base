#include "main.h"
#include "testScene.h"

#include "input.h"

#include "system.h"
#include "manager.h"
#include "titleScene.h"

#include "test.h"
#include "fpsCamera.h"
#include "skyDome.h"
#include "testParticle.h"
#include "splashParticle.h"

#include "gameDirectionalLight.h"

#include "imguiSystem.h"

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

	auto ps = new TestParticle();
	AddGameObject(ps, TYPE_TRANSPARENT);

	AddGameObject(new SplashParticle(), TYPE_TRANSPARENT);

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
	ImGui::Begin("Test Scene");
	ImGui::Text("Hello, ImGui!");
	ImGui::SliderFloat("Float Value", &m_testTimer, 0.0f, 1.0f);
	ImGui::End();
}
