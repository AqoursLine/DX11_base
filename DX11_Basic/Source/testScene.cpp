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
//#include "splashParticle.h"

#include "gameDirectionalLight.h"

#include "imguiSystem.h"
#include "testField.h"
#include "testSprite.h"

#include "testTransition.h"

#include "imguiSystem.h"

bool TestScene::Initialize() {
	//テストオブジェクト追加
	AddGameObject<TestObject>(TYPE_OPAQUE);

	//カメラ追加
	AddGameObject<FpsCamera>(TYPE_CAMERA)->SetPosition({ 0.0f, 5.0f, -20.0f });

	//スカイドーム追加
	AddGameObject<SkyDome>(TYPE_OPAQUE);

	////平行光源追加
	//Vector4 dir = { 0.0f, -1.0f, 0.01f, 0.0f };
	//AddGameObject<Light>(TYPE_LIGHT)->SetDirection(dir)
	//	->SetEnabled(true)
	//	->SetShadowCaster(true)
	//	->SetRange(200.0f)
	//	->SetPosition({ 0.0f, 20.0f, 0.0f });

	////平行光源追加
	//dir = { 1.0f, 0.0f, -1.0f, 0.0f };
	//AddGameObject<Light>(TYPE_LIGHT)->SetDirection(dir)
	//	->SetShadowCaster(true)
	//	->SetRange(200.0f)
	//	->SetEnabled(true);

	//// 点光源追加
	//AddGameObject<Light>(TYPE_LIGHT)->SetType(LIGHT_TYPE::POINT)
	//	->SetRange(100.0f)
	//	->SetIntensity(5.0f)
	//	->SetDiffuseColor({ 1.0f, 0.8f, 0.6f, 1.0f })
	//	->SetAttenuation(1.0f, 0.14f, 0.07f) // Adding back the attenuation settings
	//	->SetEnabled(true)
	//	->SetPosition({ 0.0f, 1.0f, 2.0f });


	// スポットライト追加
	AddGameObject<Light>(TYPE_LIGHT)->SetType(LIGHT_TYPE::SPOT)
		->SetRange(50.0f)
		->SetIntensity(10.0f)
		->SetDiffuseColor({ 0.6f, 0.8f, 1.0f, 1.0f })
		->SetInnerCone(XMConvertToRadians(15.0f))
		->SetOuterCone(XMConvertToRadians(30.0f))
		->SetFalloff(5.0f)
		->SetEnabled(true)
		->SetDirection({ 0.0f, -1.0f, 0.01f, 0.0f })
		->SetAttenuation(1.0f, 0.1f, 0.05f)
		->SetShadowCaster(true)
		->SetPosition({ 0.0f, 10.0f, 0.0f });


	AddGameObject<TestParticle>(TYPE_TRANSPARENT)->SetPosition({ 0.0f, 2.0f, 0.0f });

//	AddGameObject(new SplashParticle(), TYPE_TRANSPARENT);


	AddGameObject<TestField>(TYPE_OPAQUE);

	return true;
}

void TestScene::Finalize() {
}

void TestScene::Update(double deltaTime) {
	m_deltaTime = static_cast<float>(deltaTime);
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene(), new TestTransition());
	}

}

void TestScene::Draw() {
	RENDERER.ClearShadowMap(0);
	RENDERER.SetShadowMapAsRenderTarget(0);

	// fps表示
	ImGui::Begin("FPS");
	ImGui::Text("DeltaTime: %.4f", m_deltaTime);
	ImGui::Text("FPS: %.2f", 1.0f /m_deltaTime);
	ImGui::End();
}