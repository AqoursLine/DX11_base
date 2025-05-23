#pragma once
#include "gameObject.h"

class Temp2D : public GameObject {
public:
	Temp2D() : GameObject(TYPE_2D) {}
	~Temp2D() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;
	void CleanUp() override;

private:
	class Sprite* m_sprite;
};
