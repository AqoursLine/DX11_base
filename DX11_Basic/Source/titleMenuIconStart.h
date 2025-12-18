#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconStart : public TitleMenuIcon {
	public:
	TitleMenuIconStart() = default;
	~TitleMenuIconStart() = default;
	//決定処理
	void OnDecide() override;

protected:
};

