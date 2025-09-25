#include "engine.h"
#include <algorithm>

Engine::Engine()
	: m_maxPower(1000.0f)	//最大出力
	, m_currentPower(0.0f)	//現在出力
	, m_targetPower(0.0f)	//目標出力
	, m_throttle(0.0f)		//スロットル入力 (0.0f ~ 1.0f)
	, m_rpm(800.0f)			//回転数
	, m_minRPM(800.0f)		//アイドリング回転数
	, m_maxRPM(8000.0f)		//最大回転数
	, m_acceleration(2.0f)	//加速度
	, m_deceleration(9.0f)	//減速度
	, m_isRunning(true)	//エンジン稼働フラグ
{
}

void Engine::SetThrottle(float throttle) {
	m_throttle = std::clamp(throttle, 0.0f, 1.0f);

	if (m_isRunning) {
		m_targetPower = m_maxPower * m_throttle;
	} else {
		m_targetPower = 0.0f;
	}
}

void Engine::Update(double deltaTime) {
	if (!m_isRunning) {
		//エンジン停止中は出力と回転数を減少させる
		m_targetPower = 0.0f;
	}

	float dt = static_cast<float>(deltaTime);

	UpdatePower(dt);
	UpdateRPM(dt);
}

float Engine::GetEngineLoadFactor() const {
	if (!m_isRunning) {
		return 0.0f;
	}

	//スロットルとRPMを考慮した負荷率計算
	float rpmFactor = (m_rpm - m_minRPM) / (m_maxRPM - m_minRPM);
	return std::clamp(m_throttle * rpmFactor, 0.0f, 1.0f);
}

void Engine::UpdatePower(float deltaTime) {
	if (m_currentPower < m_targetPower) {
		//加速
		m_currentPower += m_acceleration * deltaTime * m_maxPower;
		m_currentPower = std::min(m_currentPower, m_targetPower);
	} else if (m_currentPower > m_targetPower) {
		//減速
		m_currentPower -= m_deceleration * deltaTime * m_maxPower;
		m_currentPower = std::max(m_currentPower, m_targetPower);
	}

	//パワーの範囲制限
	m_currentPower = std::clamp(m_currentPower, 0.0f, m_maxPower);
}

void Engine::UpdateRPM(float deltaTime) {
	//目標RPMを計算(パワーベース)
	float powerRatio = m_currentPower / m_maxPower;
	float targetRPM = m_minRPM + (m_maxRPM - m_minRPM) * powerRatio;

	//RPMを目標値に向かって変化
	float rpmChangeRate = 5.0f; //RPM変化速度
	if (m_rpm < targetRPM) {
		m_rpm += rpmChangeRate * deltaTime * (m_maxRPM - m_minRPM);
		m_rpm = std::min(m_rpm, targetRPM);
	} else if (m_rpm > targetRPM) {
		m_rpm -= rpmChangeRate * deltaTime * (m_maxRPM - m_minRPM);
		m_rpm = std::max(m_rpm, targetRPM);
	}

	//RPMの範囲制限
	m_rpm = std::clamp(m_rpm, m_minRPM, m_maxRPM);
}
