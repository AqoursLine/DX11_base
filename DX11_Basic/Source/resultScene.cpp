#include "main.h"
#include "resultScene.h"
#include "resultText.h"
#include "resultTime.h"

#include "input.h"
#include "system.h"
#include "manager.h"

#include "titleScene.h"


bool ResultScene::Initialize() {
	//タイトルテキスト初期化
	AddGameObject(new ResultText(), TYPE_BEFORE_PROCESS_UI);

	//タイムテキスト初期化
	AddGameObject(new ResultTime(), TYPE_BEFORE_PROCESS_UI);
	return true;
}

void ResultScene::Update(double deltaTime) {
	//Enterキーでシーン終了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new TitleScene());
	}
}
