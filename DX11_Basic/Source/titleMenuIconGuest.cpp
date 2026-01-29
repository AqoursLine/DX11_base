#include "main.h"
#include "titleMenuIconGuest.h"

#include "system.h"
#include "manager.h"
#include "multiWaitGuestScene.h"
#include "testTransition.h"

void TitleMenuIconGuest::OnDecide() {
	//ゲスト用待機シーンへ遷移
	SYSTEM.GetManager()->SetScene(new MultiWaitGuestScene(), new TestTransition());
}
