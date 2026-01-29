#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconGuest : public TitleMenuIcon {
	public:
	TitleMenuIconGuest() : TitleMenuIcon(L"guest.png") {}
	~TitleMenuIconGuest() = default;
	//決定処理
	void OnDecide() override;
protected:
private:
};

