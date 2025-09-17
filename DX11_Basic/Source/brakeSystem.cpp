#include "brakeSystem.h"
#include "wheel.h"
#include <algorithm>

BrakeSystem::BrakeSystem()
	: m_brakeInput(0.0f)
	, m_maxBrakeForce(8000.0f) //最大ブレーキ力(N)
	, m_frontBrakeBias(0.7f)  //前後ブレーキ力配分(0.0f～1.0f)
	, m_handbrakeEngaged(false)
	, m_absEnabled(true)
	, m_absActive(false)
	, m_absThreshold(0.1f) //ABS作動スリップ率閾値
{
	m_wheelAbs.resize(4, false); //4輪分のABS状態初期化
}

void BrakeSystem::Update(double deltaTime) {
	//ブレーキ力の更新はGetBrakeForceで行うため、ここでは特に処理しない
}

void BrakeSystem::UpdateABS(const std::vector<std::shared_ptr<Wheel>>& wheels) {
	if (!m_absEnabled || wheels.size() < 4) {
		m_absActive = false;
		return;
	}

	m_absActive = false;

	for (size_t i = 0; i < wheels.size() && i < 4; i++) {
		if (wheels[i]->IsGrounded()) {
			//簡単なスリップ率計算
			float wheelSpeed = wheels[i]->GetRotationSpeed() * wheels[i]->GetRadius();
		}
	}
}


