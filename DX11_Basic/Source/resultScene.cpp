#include "main.h"

#include "resultScene.h"
#include "resultText.h"
#include "resultTime.h"
#include "resultRankText.h"
#include "resultRankBackground.h"
#include "resultRankIcon.h"

#include "camera.h"
#include "water.h"
#include "skyDome.h"

#include "input.h"
#include "system.h"
#include "manager.h"

#include "titleScene.h"
#include "raceManager.h"

#include "testTransition.h"

#ifdef _DEBUG
#include "imguiSystem.h"
#endif // _DEBUG


bool ResultScene::Initialize() {
	//カメラ初期化
	auto camera = AddGameObject<Camera>(TYPE_CAMERA);
	camera->SetTargetPosition({ 0.0f, 2.0f, 0.0f });
	camera->SetOffset({ 0.0f, 3.0f, -10.0f });

	//水面初期化
	AddGameObject<Water>(TYPE_TRANSPARENT);

	//空ドーム初期化
	AddGameObject<SkyDome>(TYPE_OPAQUE);

	//タイトルテキスト初期化
	AddGameObject<ResultText>(TYPE_BEFORE_PROCESS_UI);

	auto resultData = RaceManager::GetResultData(); //結果データ取得

	int resultCount = static_cast<int>(resultData.size());

	//ランク背景初期化
	AddGameObject<ResultRankBackground>(TYPE_BEFORE_PROCESS_UI);

	//タイム表示初期化
	AddGameObject<ResultTime>(TYPE_BEFORE_PROCESS_UI);

	//ランキングテキスト初期化
	for (int i = 0; i < resultCount; i++) {
		AddGameObject<ResultRankText>(TYPE_BEFORE_PROCESS_UI)->SetRankIndex(i);
	}

	//ランクアイコン初期化
	AddGameObject<ResultRankIcon>(TYPE_BEFORE_PROCESS_UI);

	return true;
}

void ResultScene::Update(double deltaTime) {
	//Enterキーでシーン終了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene(), new TestTransition);
	}
}

void ResultScene::Draw() {
#ifdef _DEBUG
	// カメラの位置調整
	auto camera = GetGameObject<Camera>();
	Vector3 camPos = camera->GetPosition();
	Vector3 targetPos = camera->GetTargetPosition();
	ImGui::Begin("Camera Debug");
	ImGui::DragFloat3("Camera Position", &camPos.x, 0.1f);
	ImGui::DragFloat3("Camera Target", &targetPos.x, 0.1f);
	ImGui::End();

	camera->SetPosition(camPos);
	camera->SetTargetPosition(targetPos);

#endif // _DEBUG

}
