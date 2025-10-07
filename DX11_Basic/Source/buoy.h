#pragma once

#include "gameObject.h"

class Buoy : public GameObject {
public:
	Buoy() = default;
	virtual ~Buoy() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;
private:
	class Box* m_model = nullptr;

	float m_velocity = 0.0f;
	float m_acceleration = 0.0f;

	float m_mass = 1.0f; //質量(kg)

	class Water* m_water = nullptr;

	//浮力計算
	float CalculateBuoyancy() const;
	//沈んだ体積計算
	float CalculateSubmergedVolume() const;
	//抵抗
	float CalculateDrag() const;
};
