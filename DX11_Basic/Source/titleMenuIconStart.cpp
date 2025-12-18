#include "titleMenuIconStart.h"

#include "system.h"
#include "manager.h"
#include "gameScene.h"
#include "testTransition.h"


void TitleMenuIconStart::OnDecide() {
	SYSTEM.GetManager()->SetScene(new GameScene(), new TestTransition());
}
