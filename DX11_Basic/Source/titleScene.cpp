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

#include "titleMenuIconSingle.h"
#include "titleMenuIconMulti.h"

#include "titleMenuIconHost.h"
#include "titleMenuIconGuest.h"


bool TitleScene::Initialize() {
	//タイトル背景ムービー初期化
	AddGameObject<TitleBackgroundMove>(TYPE_BEFORE_PROCESS_UI);

	//タイトルメニューバックグラウンド初期化
	AddGameObject<TitleManuBackground>(TYPE_BEFORE_PROCESS_UI);

	//タイトルメニューセレクター初期化
	TitleMenuSelecter* selecter = AddGameObject<TitleMenuSelecter>(TYPE_BEFORE_PROCESS_UI);

	// アイコン位置
	Vector3 iconPosition = { SCREEN_WIDTH - 50.0f, SCREEN_HEIGHT * 0.5f, 0.0f };

	//タイトルメニュースタートアイコン初期化
	auto startIcon = AddGameObject<TitleMenuIconStart>(TYPE_BEFORE_PROCESS_UI);
	startIcon->SetPosition(iconPosition);
	selecter->AddMenuIcon(startIcon);

	//タイトルメニューシングルアイコン初期化
	AddGameObject<TitleMenuIconSingle>(TYPE_BEFORE_PROCESS_UI)->SetPosition(iconPosition)->SetActive(false);

	//タイトルメニューホストアイコン初期化
	AddGameObject<TitleMenuIconHost>(TYPE_BEFORE_PROCESS_UI)->SetPosition(iconPosition)->SetActive(false);

	//タイトルメニュー終了アイコン初期化
	//アイコン位置を少し下にずらす
	iconPosition.y += 200.0f;
	auto quitIcon = AddGameObject<TitleMenuIconQuit>(TYPE_BEFORE_PROCESS_UI);
	quitIcon->SetPosition(iconPosition);
	selecter->AddMenuIcon(quitIcon);

	//タイトルメニューマルチアイコン初期化
	AddGameObject<TitleMenuIconMulti>(TYPE_BEFORE_PROCESS_UI)->SetPosition(iconPosition)->SetActive(false);

	//タイトルメニューゲストアイコン初期化
	AddGameObject<TitleMenuIconGuest>(TYPE_BEFORE_PROCESS_UI)->SetPosition(iconPosition)->SetActive(false);

	//タイトルテキスト初期化
	AddGameObject<TitleText>(TYPE_BEFORE_PROCESS_UI);

    return true;
}

void TitleScene::Update(double deltaTime) {
}
