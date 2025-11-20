#include "main.h"
#include "gameScene.h"

#include "tmp2D.h"
#include "fieldObject.h"
#include "player.h"
#include "camera.h"
#include "fpsCamera.h"
#include "water.h"
#include "buoy.h"
#include "skyDome.h"
#include "raceManager.h"
#include "countDownText.h"
#include "minimap.h"
#include "speedMeter.h"
#include "wall.h"
#include "startGate.h"
#include "goalGate.h"
#include "lapReadyGate.h"
#include "raceTimer.h"
#include "lapDisplay.h"
#include "test.h"
#include "gameDirectionalLight.h"

#include "cpuBoat.h"

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

	//CPUボート
	AddGameObject(new CPUBoat(), TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -10.0f});
	AddGameObject(new CPUBoat(), TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -20.0f });
	AddGameObject(new CPUBoat(), TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -30.0f });
	AddGameObject(new CPUBoat(), TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -40.0f });
	AddGameObject(new CPUBoat(), TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -50.0f });

	//浮き
	auto bob1 = AddGameObject(new Buoy(), TYPE_OPAQUE);
	bob1->SetPosition({ 150.0f, 0.0f, 0.0f });
	auto bob2 = AddGameObject(new Buoy(), TYPE_OPAQUE);
	bob2->SetPosition({ -150.0f, 0.0f, 0.0f });

	//レースマネージャー
	AddGameObject(new RaceManager(), TYPE_CUTOUT);

	//カウントダウン
	AddGameObject(new RaceCountDownText(), TYPE_BEFORE_PROCESS_UI);

	//ミニマップ
	AddGameObject(new MiniMap(), TYPE_BEFORE_PROCESS_UI);

	//スピードメーター
	AddGameObject(new SpeedMeter(), TYPE_BEFORE_PROCESS_UI);

	//スタートゲート
	AddGameObject(new StartGate(), TYPE_CUTOUT);

	//ゴールゲート
	AddGameObject(new GoalGate(), TYPE_CUTOUT);

	//ラップゲート
	AddGameObject(new LapReadyGate(), TYPE_CUTOUT);

	//レースタイマー
	AddGameObject(new RaceTimer(), TYPE_BEFORE_PROCESS_UI);

	//ラップ表示
	AddGameObject(new LapDisplay(), TYPE_BEFORE_PROCESS_UI);

	//カメラの初期化
	AddGameObject(new Camera(), TYPE_CAMERA);
	//	AddGameObject(new FpsCamera(), TYPE_CAMERA);

	//スカイドーム
	AddGameObject(new SkyDome(), TYPE_OPAQUE);

	//平行光源
	Light* light = new GameDirectionalLight();
	light->SetDirection({ -1.0f, -1.0f, -1.0f, 0.0f });
	light->SetEnabled(true);
	AddGameObject(light, TYPE_LIGHT);

	return true;
}
