#include "main.h"
#include "titleMenuIconSingle.h"

#include "system.h"
#include "manager.h"
#include "gameScene.h"
#include "testTransition.h"

void TitleMenuIconSingle::OnDecide() {
	SYSTEM.GetManager()->SetScene(new GameScene(), new TestTransition());
}
