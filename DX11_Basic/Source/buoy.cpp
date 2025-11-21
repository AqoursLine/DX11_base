#include "buoy.h"
#include "box.h"
#include "system.h"
#include "manager.h"
#include "scene.h"
#include "water.h"

#ifdef _DEBUG
#include <iostream>

#endif // _DEBUG


bool Buoy::Initialize() {
	m_model = new Box();
	if (!m_model->Initialize()) {
		return false;
	}

	m_rotation = { 0.0f, 0.0f, 0.0f };
	m_scale = { 1.1f, 0.95f, 1.1f };

	m_water = m_scene->GetGameObject<Water>();

	return true;
}

void Buoy::Finalize() {
	if (m_model) {
		m_model->Finalize();
		delete m_model;
	}
}

void Buoy::Update(double deltaTime) {
	//重力
	float gravity = 9.81f * m_mass;

	//浮力
	float buoyancy = CalculateBuoyancy();

	//抵抗
	float drag = CalculateDrag();

	//合力から加速度を計算
	float totalForce = buoyancy - gravity - drag;
	m_acceleration = totalForce / m_mass;

	//速度と位置を更新
	float dt = static_cast<float>(deltaTime);
	m_velocity += m_acceleration * dt;

	//位置更新
//	m_position.y = m_water->GetWaterHeight(m_position);
	m_position.y += m_velocity * dt;

	//デバッグ表示
}

void Buoy::Draw() {
	if (m_model) {
		m_model->Draw(m_position, m_rotation, m_scale);
	}
}

float Buoy::CalculateBuoyancy() const {
	//沈んだ高さを計算
	float waterHeight = m_water->GetWaterHeight(m_position);
	float submergedHeight = waterHeight - (m_position.y - m_scale.y);

	if (submergedHeight <= 0.0f) {
		//浮いている場合、浮力はゼロ
		return 0.0f;
	}

	//沈んだ分だけ浮力を計算
	return submergedHeight * m_mass * 9.81f * 1.8f; //簡略化した浮力計算
}


float Buoy::CalculateDrag() const {
	//水の抵抗(簡略化)
	float dragCoefficient = 0.5f; //抵抗係数
	return dragCoefficient * m_velocity; //抵抗力 = 0.5 * v^2
}
