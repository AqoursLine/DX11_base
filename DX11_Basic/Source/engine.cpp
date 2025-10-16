#include <algorithm>
#include "engine.h"
#include <iostream>

Engine::Engine()
	: m_maxPower(100.0f)	//最大出力(kw)
	, m_currentPower(0.0f)	//現在出力
	, m_currentTorque(0.0f)	//現在トルク(Nm)
	, m_throttle(0.0f)		//スロットル入力 (0.0f ~ 1.0f)
	, m_rpm(800.0f)			//回転数
	, m_targetRPM(800.0f)	//目標回転数
	, m_minRPM(800.0f)		//アイドリング回転数
	, m_maxRPM(8000.0f)		//最大回転数
	, m_rpmAcceleration(1000.0f)	//RPM加速度(RPM/s)
	, m_rpmDeceleration(500.0f)	//RPM減速度(RPM/s)
	, m_engineLoad(0.0f)			//エンジン負荷
	, m_loadSensitivity(0.7f)		//負荷感度
	, m_isRunning(true)	//エンジン稼働フラグ
{
}

void Engine::SetThrottle(float throttle) {
	m_throttle = std::clamp(throttle, 0.0f, 1.0f);

	//スロットルから目標RPMを計算
	if (m_isRunning) {
		m_targetRPM = m_minRPM + (m_maxRPM - m_minRPM) * m_throttle;
	} else {
		m_targetRPM = m_minRPM; //エンジン停止中はアイドリング回転数
	}
}

void Engine::SetEngineLoad(float velocity, float resistance) {
	//エンジン負荷を計算(0.0 = 負荷無し, 1.0 = 最大負荷)
	//負荷は速度と抵抗に依存

	if (velocity < 1.0f) {
		m_engineLoad = 0.0f;
		return;
	}

	//必要なパワーから負荷を計算
	float requiredPower = (velocity * resistance) / 1000.0f; //kw
	m_engineLoad = std::clamp(requiredPower / m_maxPower, 0.0f, 1.0f);
}

void Engine::Update(double deltaTime) {
	if (!m_isRunning) {
		//エンジン停止中は最小RPMを目標にする
		m_targetRPM = m_minRPM;
	}

	float dt = static_cast<float>(deltaTime);

	//RPM更新
	UpdateRPM(dt);

	//出力更新
	UpdatePowerFromRPM();
}


float Engine::GetEngineLoadFactor() const {
	if (!m_isRunning) {
		return 0.0f;
	}

	//スロットルとRPMを考慮した負荷率計算
	float rpmFactor = (m_rpm - m_minRPM) / (m_maxRPM - m_minRPM);
	return std::clamp(m_throttle * rpmFactor, 0.0f, 1.0f);
}

void Engine::UpdateRPM(float deltaTime) {
	//目標RPMに向かってRPMを変化
	//負荷が高いほどRPMの上昇が抑制される
	float actualTargetRPM = std::min(m_targetRPM, m_minRPM + (m_maxRPM - m_minRPM) * (1.0f - m_engineLoad * m_loadSensitivity));

	float loadMultiplier = 1.0f;
	if (m_rpm < actualTargetRPM) {
		//加速(負荷が高いほど加速が遅くなる)
		loadMultiplier = 1.0f - (m_engineLoad * m_loadSensitivity);
		m_rpm += m_rpmAcceleration * loadMultiplier * deltaTime;
		m_rpm = std::min(m_rpm, actualTargetRPM);

	} else if (m_rpm > actualTargetRPM) {
		//減速
		m_rpm -= m_rpmDeceleration * deltaTime;
		m_rpm = std::max(m_rpm, actualTargetRPM);
	}

	//RPM制限
	m_rpm = std::clamp(m_rpm, m_minRPM, m_maxRPM);
}

void Engine::UpdatePowerFromRPM() {
	//現在のトルクをトルクカーブから取得
	m_currentTorque = GetTorqueAtRPM(m_rpm);
	//現在の出力を計算
	m_currentPower = CalculatePower(m_currentTorque, m_rpm); //kw
}

float Engine::GetTorqueAtRPM(float rpm) const {
	if (m_torqueCurve.empty()) {
		return 0.0f;
	}

	//トルクカーブの範囲外
	if (rpm <= m_torqueCurve.front().rpm) {
		return m_torqueCurve.front().torque;
	}
	if (rpm >= m_torqueCurve.back().rpm) {
		return m_torqueCurve.back().torque;
	}

	//トルクカーブ内を線形補間
	for (size_t i = 0; i < m_torqueCurve.size() - 1; ++i) {
		const TorquePoint& p1 = m_torqueCurve[i];
		const TorquePoint& p2 = m_torqueCurve[i + 1];
		if (rpm >= p1.rpm && rpm <= p2.rpm) {
			float t = (rpm - p1.rpm) / (p2.rpm - p1.rpm);
			return p1.torque + t * (p2.torque - p1.torque);
		}
	}

	return 0.0f; //理論上ここには来ない
}

float Engine::CalculatePower(float torque, float rpm) const {
	//パワー(kW) = トルク(Nm) * 角速度(rad/s) / 1000
	// 角速度(rad/s) = RPM * (2 * π / 60)
	float angularVelocity = rpm * (2.0f * 3.14159265f / 60.0f);
	return (torque * angularVelocity) / 1000.0f;
}
