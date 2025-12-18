#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconQuit : public TitleMenuIcon {
public:
	TitleMenuIconQuit() = default;
	~TitleMenuIconQuit() = default;
	//決定処理
	void OnDecide() override;
};
