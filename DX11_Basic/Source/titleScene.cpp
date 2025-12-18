#include "main.h"
#include "titleScene.h"
#include "titleText.h"
#include "input.h"
#include "system.h"
#include "manager.h"
#include "gameScene.h"

#include "testTransition.h"
#include "titleBackgroundMove.h"

#include "titleMenuBackground.h"
#include "titleMenuSelecter.h"
#include "titleMenuIconStart.h"
#include "titleMenuIconQuit.h"


bool TitleScene::Initialize() {
	//タイトル背景ムービー初期化
	AddGameObject(new TitleBackgroundMove(), TYPE_BEFORE_PROCESS_UI);

	//タイトルメニューバックグラウンド初期化
	AddGameObject(new TitleManuBackground(), TYPE_BEFORE_PROCESS_UI);

	//タイトルメニューセレクター初期化
	AddGameObject(new TitleMenuSelecter(), TYPE_BEFORE_PROCESS_UI);

	// アイコン位置
	Vector3 iconPosition = { SCREEN_WIDTH - 50.0f, SCREEN_HEIGHT * 0.5f, 0.0f };

	//タイトルメニュースタートアイコン初期化
	AddGameObject(new TitleMenuIconStart(), TYPE_BEFORE_PROCESS_UI)->SetPosition(iconPosition);

	//タイトルメニュー終了アイコン初期化
	//アイコン位置を少し下にずらす
	iconPosition.y += 200.0f;
	AddGameObject(new TitleMenuIconQuit(), TYPE_BEFORE_PROCESS_UI)->SetPosition(iconPosition);



	//タイトルテキスト初期化
	AddGameObject(new TitleText(), TYPE_BEFORE_PROCESS_UI);


    return true;
}

void TitleScene::Update(double deltaTime) {
}
