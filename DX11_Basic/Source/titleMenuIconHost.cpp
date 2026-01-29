#include "main.h"
#include "titleMenuIconHost.h"

#include "system.h"
#include "manager.h"
#include "gameScene.h"
#include "testTransition.h"

#include "multiWaitHostScene.h"

void TitleMenuIconHost::OnDecide() {
	//ホスト用待機シーンへ遷移
	SYSTEM.GetManager()->SetScene(new MultiWaitHostScene(), new TestTransition());
}
