#include "main.h"
#include "gameScene.h"

#include "tmp2D.h"
#include "fieldObject.h"
#include "player.h"
#include "camera.h"
#include "fpsCamera.h"
#include "raceCourseManager.h"
#include "water.h"
#include "buoy.h"
#include "skyDome.h"
#include "raceManager.h"
#include "countDownText.h"
#include "minimap.h"
#include "speedMeter.h"
#include "wall.h"
#include "startGate.h"
#include "raceTimer.h"

bool GameScene::Initialize() {
	//とりあえずの壁
	//右壁
	AddGameObject(new Wall(), TYPE_OPAQUE)->SetPosition({ 220.0f, 5.0f, 0.0f })->SetScale({ 10.0f, 20.0f, 130.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//左壁
	AddGameObject(new Wall(), TYPE_OPAQUE)->SetPosition({ -220.0f, 5.0f, 0.0f })->SetScale({ 10.0f, 20.0f, 130.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//前壁
	AddGameObject(new Wall(), TYPE_OPAQUE)->SetPosition({ 0.0f, 5.0f, 65.0f })->SetScale({ 400.0f, 20.0f, 10.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//後壁
	AddGameObject(new Wall(), TYPE_OPAQUE)->SetPosition({ 0.0f, 5.0f, -65.0f })->SetScale({ 400.0f, 20.0f, 10.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });

	//ゆきのん初期化
	AddGameObject(new Temp2D(), TYPE_BEFORE_PROCESS_UI);

	//フィールドオブジェクト
	AddGameObject(new FieldObject(), TYPE_OPAQUE);

	//水
	AddGameObject(new Water(), TYPE_TRANSPARENT);

	//プレイヤー
	AddGameObject(new Player(), TYPE_OPAQUE);

	//浮き
	auto bob1 = AddGameObject(new Buoy(), TYPE_OPAQUE);
	bob1->SetPosition({ 150.0f, 0.0f, 0.0f });
	auto bob2 = AddGameObject(new Buoy(), TYPE_OPAQUE);
	bob2->SetPosition({ -150.0f, 0.0f, 00.0f });

	//コースマネージャー
	AddGameObject(new RaceCourseManager(), TYPE_OPAQUE);

	//レースマネージャー
	AddGameObject(new RaceManager(), TYPE_OPAQUE);

	//カウントダウン
	AddGameObject(new RaceCountDownText(), TYPE_BEFORE_PROCESS_UI);

	//ミニマップ
	AddGameObject(new MiniMap(), TYPE_BEFORE_PROCESS_UI);

	//スピードメーター
	AddGameObject(new SpeedMeter(), TYPE_BEFORE_PROCESS_UI);

	//スタートゲート
	AddGameObject(new StartGate(), TYPE_CUTOUT);

	//レースタイマー
	AddGameObject(new RaceTimer(), TYPE_BEFORE_PROCESS_UI);

	//カメラの初期化
	AddGameObject(new Camera(), TYPE_CAMERA);
	//	AddGameObject(new FpsCamera(), TYPE_CAMERA);

		//スカイドーム
	//	AddGameObject(new SkyDome(), TYPE_3D);

	return true;
}
