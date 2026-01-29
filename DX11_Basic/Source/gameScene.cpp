#include "main.h"
#include "gameScene.h"

#include "tmp2D.h"
#include "fieldObject.h"
#include "player.h"
#include "camera.h"
#include "fpsCamera.h"
#include "firstFollowCamera.h"
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
#include "splashParticle.h"
#include "cpuBoat.h"
#include "rankDisplay.h"

bool GameScene::Initialize() {
	//とりあえずの壁
	//右壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ 220.0f, 5.0f, 0.0f })->SetScale({ 10.0f, 20.0f, 130.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//左壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ -220.0f, 5.0f, 0.0f })->SetScale({ 10.0f, 20.0f, 130.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//前壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ 0.0f, 5.0f, 65.0f })->SetScale({ 400.0f, 20.0f, 10.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//後壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ 0.0f, 5.0f, -65.0f })->SetScale({ 400.0f, 20.0f, 10.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//ゆきのん初期化
//	AddGameObject(new Temp2D(), TYPE_BEFORE_PROCESS_UI);

	//フィールドオブジェクト
	AddGameObject<FieldObject>(TYPE_OPAQUE);

	//水
	AddGameObject<Water>(TYPE_TRANSPARENT);

	//プレイヤー
	AddGameObject<Player>(TYPE_OPAQUE);

	//CPUボート
	AddGameObject<CPUBoat>(TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -10.0f});
	AddGameObject<CPUBoat>(TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -20.0f });
	AddGameObject<CPUBoat>(TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -30.0f });
	AddGameObject<CPUBoat>(TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -40.0f });
	AddGameObject<CPUBoat>(TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -50.0f });
//	AddGameObject<CPUBoat>(TYPE_OPAQUE)->SetPosition({ -160.0f, 0.0f, -60.0f });

	//浮き
	auto bob1 = AddGameObject<Buoy>(TYPE_OPAQUE);
	bob1->SetPosition({ 150.0f, 0.0f, 0.0f });
	auto bob2 = AddGameObject<Buoy>(TYPE_OPAQUE);
	bob2->SetPosition({ -150.0f, 0.0f, 0.0f });

	//レースマネージャー
	AddGameObject<RaceManager>(TYPE_CUTOUT);
	//カウントダウン
	AddGameObject<RaceCountDownText>(TYPE_BEFORE_PROCESS_UI);

	//ミニマップ
	AddGameObject<MiniMap>(TYPE_BEFORE_PROCESS_UI);

	//スピードメーター
	AddGameObject<SpeedMeter>(TYPE_BEFORE_PROCESS_UI);

	//スタートゲート
	AddGameObject<StartGate>(TYPE_CUTOUT);

	//ゴールゲート
	AddGameObject<GoalGate>(TYPE_CUTOUT);

	//ラップゲート
	AddGameObject<LapReadyGate>(TYPE_CUTOUT);

	//レースタイマー
	AddGameObject<RaceTimer>(TYPE_BEFORE_PROCESS_UI);

	//ラップ表示
	AddGameObject<LapDisplay>(TYPE_BEFORE_PROCESS_UI);

	//順位表示
	AddGameObject<RankDisplay>(TYPE_BEFORE_PROCESS_UI);

	//カメラの初期化
	AddGameObject<Camera>(TYPE_CAMERA);
//	AddGameObject<FpsCamera>(TYPE_CAMERA);
//	AddGameObject<FirstFollowCamera>(TYPE_CAMERA);

	//スカイドーム
	AddGameObject<SkyDome>(TYPE_OPAQUE);

	//平行光源
	AddGameObject<GameDirectionalLight>(TYPE_LIGHT)->SetDirection({ -1.0f, -1.0f, -1.0f, 0.0f })->SetEnabled(true);

	//スプラッシュエフェクト
	AddGameObject<SplashParticle>(TYPE_TRANSPARENT);
	return true;
}
