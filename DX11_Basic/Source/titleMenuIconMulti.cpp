#include "main.h"
#include "titleMenuIconMulti.h"

#include "scene.h"
#include "titleMenuSelecter.h"
#include "titleMenuIconHost.h"
#include "titleMenuIconGuest.h"

void TitleMenuIconMulti::OnDecide() {
	m_menuSelecter->ClearMenuIcons();
	for (auto& icon : m_menuIcons) {
		icon->SetActive(true);
		m_menuSelecter->AddMenuIcon(icon);
	}
	m_menuIcons[0]->IsSelected(true);
}

bool TitleMenuIconMulti::Initialize() {
	// メニューセレクター取得
	m_menuSelecter = m_scene->GetGameObject<TitleMenuSelecter>();
	if (!m_menuSelecter) {
		return false;
	}

	// シングルプレイアイコン追加
	auto singleIcon = m_scene->GetGameObject<TitleMenuIconHost>();
	m_menuIcons.push_back(singleIcon);

	// マルチプレイアイコン追加
	auto multiIcon = m_scene->GetGameObject<TitleMenuIconGuest>();
	m_menuIcons.push_back(multiIcon);

	return TitleMenuIcon::Initialize();
}
