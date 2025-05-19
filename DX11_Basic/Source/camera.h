#pragma once

class Camera {
public:
	Camera() = default;
	~Camera() = default;

	virtual void Initialize();
	virtual void Finalize();
	virtual void Update(double deltaTime);
	virtual void Draw() const;

protected:
	void CalculateVector();

	//¶‰EˆÚ“®ŠÖ”
	void MoveSide(bool isRight, double deltaTime);

	//‘OŒãˆÚ“®ŠÖ”
	void MoveForward(bool isForward, double deltaTime);

	//ˆÚ“®—Ê
	float m_moveSpeed = 3.0f;
	//‰ñ“]—Ê
	float m_rotateSpeed = 0.5f;

private:
	Vector3 m_position { 0.0f, 0.3f, -5.0f };
	Vector3 m_rotation { 0.0f, 0.0f, 0.0f };

	Vector3 m_up { 0.0f, 1.0f, 0.0f };
	Vector3 m_right { 1.0f, 0.0f, 0.0f };
	Vector3 m_forward { 0.0f, 0.0f, 1.0f };
};
