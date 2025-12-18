#pragma once

#include "gameObject.h"

class TitleMenuIcon;

class TitleMenuSelecter : public GameObject {
public:
	TitleMenuSelecter() = default;
	~TitleMenuSelecter() = default;

protected:
	bool Initialize() override;
	void Update(double deltaTime) override;

private:
	std::vector<TitleMenuIcon*> m_menuIcons;

	int m_currentIndex = 0;
	int m_previousIndex = 0;
	int m_maxIndex = 0;
};
