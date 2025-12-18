#include "titleMenuSelecter.h"
#include "scene.h"
#include "titleMenuIcon.h"

#include "input.h"

bool TitleMenuSelecter::Initialize() {
	m_menuIcons.clear();

	m_menuIcons = m_scene->GetGameObjects<TitleMenuIcon>();

	m_maxIndex = static_cast<int>(m_menuIcons.size());

	if (m_maxIndex == 0) {
		SetActive(false);
		return false;
	}

	// 最初のメニューアイコンを選択状態にする
	m_menuIcons[m_currentIndex]->IsSelected(true);
	return true;
}

void TitleMenuSelecter::Update(double deltaTime) {
	// 上下入力でメニュー移動
	if (Input::GetKeyTrigger(KK_W) || Input::GetKeyTrigger(KK_UP)) {
		m_previousIndex = m_currentIndex;
		m_currentIndex = (m_currentIndex - 1 + m_maxIndex) % m_maxIndex;
	}
	if (Input::GetKeyTrigger(KK_S) || Input::GetKeyTrigger(KK_DOWN)) {
		m_previousIndex = m_currentIndex;
		m_currentIndex = (m_currentIndex + 1) % m_maxIndex;
	}

	// 選択状態更新
	if (m_previousIndex != m_currentIndex) {
		m_menuIcons[m_previousIndex]->IsSelected(false);
		m_menuIcons[m_currentIndex]->IsSelected(true);
		m_previousIndex = m_currentIndex;
	}

	// 決定入力でメニュー決定
	if (Input::GetKeyTrigger(KK_ENTER) || Input::GetKeyTrigger(KK_SPACE)) {
		m_menuIcons[m_currentIndex]->OnDecide();
	}

}
