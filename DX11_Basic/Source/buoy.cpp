#include "buoy.h"
#include "box.h"
#include "system.h"
#include "manager.h"
#include "scene.h"
#include "water.h"

bool Buoy::Initialize() {
	m_model = new Box();
	if (!m_model->Initialize()) {
		return false;
	}

	m_rotation = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.0f, 1.0f, 1.0f };

	m_water = SYSTEM.GetManager()->GetScene()->GetGameObject<Water>();

	return true;
}

void Buoy::Finalize() {
	if (m_model) {
		m_model->Finalize();
		delete m_model;
	}
}

void Buoy::Update(double deltaTime) {
	////重力
	//float gravity = 9.81f * m_mass;

	////浮力
	//float buoyancy = CalculateBuoyancy();

	////抵抗
	//float drag = CalculateDrag();

	////合力から加速度を計算
	//float totalForce = buoyancy - gravity - drag;
	//m_acceleration = totalForce / m_mass;

	////速度と位置を更新
	//float dt = static_cast<float>(deltaTime);
	//m_velocity += m_acceleration * dt;

	//位置更新
	m_position.y = m_water->GetWaterHeight(m_position);
}

void Buoy::Draw() const {
	if (m_model) {
		m_model->Draw(m_position, m_rotation, m_scale);
	}
}

float Buoy::CalculateBuoyancy() const {
	//水の密度(kg/m^3)
	float waterDensity = 1000.0f;
	float g = 9.81f; //重力加速度(m/s^2)

	//水中に沈んだ体積(m^3)
	float submergedVolume = CalculateSubmergedVolume();

	//浮力(F = ρ * g * V)
	return waterDensity * g * submergedVolume;
}

float Buoy::CalculateSubmergedVolume() const {
	//体積
	float volume = m_scale.x * m_scale.y * m_scale.z; //立方体として計算

	//水面の高さ
	float waterHeight = m_water->GetWaterHeight(m_position);
//	float waterHeight = 0.0f; //静止水面をy=0と仮定

	//浮きの底面のy座標
	float bobberBottomY = m_position.y - (m_scale.y * 0.5f);

	if (bobberBottomY >= waterHeight) {
		//完全に水面より上
		return 0.0f;
	} else if (m_position.y + (m_scale.y * 0.5f) <= waterHeight) {
		//完全に水面より下
		return volume;
	} else {
		//一部が水中に沈んでいる場合
		float submergedHeight = waterHeight - bobberBottomY;
		float submergedRatio = submergedHeight / m_scale.y;
		return submergedRatio * volume;
	}
}

float Buoy::CalculateDrag() const {
	//水の抵抗(簡略化)
	float dragCoefficient = 0.5f; //抵抗係数
	return dragCoefficient * m_velocity;
}
