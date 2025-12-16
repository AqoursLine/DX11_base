#include "main.h"
#include "titleScene.h"
#include "titleText.h"
#include "input.h"
#include "system.h"
#include "manager.h"
#include "gameScene.h"

#include "testTransition.h"
#include "titleBackgroundMove.h"

bool TitleScene::Initialize() {
	//タイトル背景ムービー初期化
	AddGameObject(new TitleBackgroundMove(), TYPE_BEFORE_PROCESS_UI);

	//タイトルテキスト初期化
	AddGameObject(new TitleText(), TYPE_BEFORE_PROCESS_UI);

    return true;
}

void TitleScene::Update(double deltaTime) {
	//Enterキーでシーン終了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new GameScene(), new TestTransition());
	}
}
