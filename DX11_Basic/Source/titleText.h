#pragma once

#include "gameObject.h"

class TitleText : public GameObject {
public:
	TitleText() = default;
	~TitleText() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Sprite* m_sprite = nullptr;
};
