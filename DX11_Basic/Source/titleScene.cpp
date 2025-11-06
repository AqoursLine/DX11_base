#include "main.h"
#include "titleScene.h"
#include "titleText.h"
#include "input.h"
#include "system.h"
#include "manager.h"
#include "gameScene.h"

bool TitleScene::Initialize() {
	//タイトルテキスト初期化
	AddGameObject(new TitleText(), TYPE_BEFORE_PROCESS_UI);
    return true;
}

void TitleScene::Update(double deltaTime) {
	//Enterキーでシーン終了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		SYSTEM.GetManager()->SetScene(new GameScene());
	}
}
