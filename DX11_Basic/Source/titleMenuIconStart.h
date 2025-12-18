#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconStart : public TitleMenuIcon {
	public:
	TitleMenuIconStart() : TitleMenuIcon(L"start.png") {}
	~TitleMenuIconStart() = default;
	//決定処理
	void OnDecide() override;

protected:
};

