#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconMulti : public TitleMenuIcon {
	public:
	TitleMenuIconMulti() : TitleMenuIcon(L"multi.png") {}
	~TitleMenuIconMulti() = default;
	//決定処理
	void OnDecide() override;
protected:
	bool Initialize() override;
private:
	std::vector<TitleMenuIcon*> m_menuIcons;
	class TitleMenuSelecter* m_menuSelecter = nullptr;

};

