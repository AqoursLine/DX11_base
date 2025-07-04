#pragma once

#include "gameObject.h"

class Ball : public GameObject {
public:
	Ball() = default;
	~Ball() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Model* m_model = nullptr;
	class btRigidBody* m_body = nullptr; // 物理オブジェクトのポインタ

};
