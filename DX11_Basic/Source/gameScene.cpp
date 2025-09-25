#include "gameScene.h"

#include "tmp2D.h"
#include "fieldObject.h"
#include "player.h"
#include "camera.h"
#include "fpsCamera.h"
#include "raceCourseManager.h"
#include "water.h"
#include "bobber.h"
#include "skyDome.h"

bool GameScene::Initialize() {

	//ゆきのん初期化
	AddGameObject(new Temp2D(), TYPE_2D);

	//フィールドオブジェクト
	AddGameObject(new FieldObject(), TYPE_3D);

	//水
	AddGameObject(new Water(), TYPE_3D);

	//プレイヤー
	AddGameObject(new Player(), TYPE_3D);

	//浮き
	AddGameObject(new Bobber(), TYPE_3D);

	//コースマネージャー
	AddGameObject(new RaceCourseManager(), TYPE_3D);

	//カメラの初期化
	AddGameObject(new Camera(), TYPE_CAMERA);
	//	AddGameObject(new FpsCamera(), TYPE_CAMERA);

		//スカイドーム
	//	AddGameObject(new SkyDome(), TYPE_3D);
	return true;
}
