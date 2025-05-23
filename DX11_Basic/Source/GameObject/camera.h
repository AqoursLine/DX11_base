#pragma once

#include "gameObject.h"

class Camera : public GameObject {
public:
	Camera() : GameObject(TYPE_CAMERA) {}
	~Camera() = default;

	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;
	virtual void CleanUp() override;

protected:
	void CalculateVector();

	//¶‰EˆÚ“®ŠÖ”
	void MoveSide(bool isRight, double deltaTime);

	//‘OŒãˆÚ“®ŠÖ”
	void MoveForward(bool isForward, double deltaTime);

	//ˆÚ“®—Ê
	float m_moveSpeed = 5.0f;
	//‰ñ“]—Ê
	float m_rotateSpeed = 0.5f;

private:
	Vector3 m_up { 0.0f, 1.0f, 0.0f };
	Vector3 m_right { 1.0f, 0.0f, 0.0f };
	Vector3 m_forward { 0.0f, 0.0f, 1.0f };
};
