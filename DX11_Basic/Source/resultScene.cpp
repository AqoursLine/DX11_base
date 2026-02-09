#include "main.h"
#include "resultScene.h"
#include "resultText.h"
#include "resultTime.h"
#include "resultRankText.h"
#include "resultRankBackground.h"

#include "input.h"
#include "system.h"
#include "manager.h"

#include "titleScene.h"
#include "raceManager.h"

#include "testTransition.h"

bool ResultScene::Initialize() {
	//タイトルテキスト初期化
	AddGameObject<ResultText>(TYPE_BEFORE_PROCESS_UI);

	auto resultData = RaceManager::GetResultData(); //結果データ取得

	int resultCount = static_cast<int>(resultData.size());

	//タイム表示初期化
	for (int i = 0; i < resultCount; i++) {
		AddGameObject<ResultTime>(TYPE_BEFORE_PROCESS_UI)->SetResultCount(resultCount)->SetResultData(resultData[i])->SetIndex(i);
	}

	//ランキングテキスト初期化
	for (int i = 0; i < resultCount; i++) {
		AddGameObject<ResultRankText>(TYPE_BEFORE_PROCESS_UI)->SetRankIndex(i);
	}

	return true;
}

void ResultScene::Update(double deltaTime) {
	//Enterキーでシーン終了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene(), new TestTransition);
	}
}
