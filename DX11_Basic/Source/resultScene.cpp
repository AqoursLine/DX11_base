#include "main.h"
#include "resultScene.h"
#include "resultText.h"
#include "resultTime.h"

#include "input.h"
#include "system.h"
#include "manager.h"

#include "titleScene.h"
#include "raceManager.h"

#include "testTransition.h"

bool ResultScene::Initialize() {
	//タイトルテキスト初期化
	AddGameObject(new ResultText(), TYPE_BEFORE_PROCESS_UI);

	auto resultData = RaceManager::GetResultData(); //結果データ取得

	int resultCount = static_cast<int>(resultData.size());

	//タイム表示初期化
	for (int i = 0; i < resultCount; i++) {
		AddGameObject(new ResultTime(resultCount, resultData[i], i), TYPE_BEFORE_PROCESS_UI);
	}

	return true;
}

void ResultScene::Update(double deltaTime) {
	//Enterキーでシーン終了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene(), new TestTransition);
	}
}
