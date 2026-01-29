#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconSingle : public TitleMenuIcon {
public:
	TitleMenuIconSingle() : TitleMenuIcon(L"single.png") {}
	~TitleMenuIconSingle() = default;
	//決定処理
	void OnDecide() override;

protected:

private:
};

