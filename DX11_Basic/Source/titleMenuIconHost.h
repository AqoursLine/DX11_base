#pragma once

#include "titleMenuIcon.h"
class TitleMenuIconHost : public TitleMenuIcon {
	public:
	TitleMenuIconHost() : TitleMenuIcon(L"host.png") {}
	~TitleMenuIconHost() = default;
	//決定処理
	void OnDecide() override;

protected:

private:
};

