#include "engine.h"
#include <algorithm>
#include <cmath>

Engine::Engine()
	: m_maxPower(300.0f)	//最大出力(馬力)
	, m_maxTorque(400.0f)	//最大トルク(Nm)
	, m_maxRPM(7000.0f)		//最大回転数(rpm)
	, m_idleRPM(800.0f)		//アイドリング回転数(rpm)
	, m_redlineRPM(6500.0f)	//レッドライン回転数(rpm)
	, m_currentRPM(0.0f)
	, m_currentPower(0.0f)
	, m_currentTorque(0.0f)
	, m_throttle(0.0f)
	, m_brake(0.0f)
	, m_temperature(90.0f)
	, m_maxTemperature(120.0f) //最大許容温度(℃)
	, m_coolingRate(0.5f)	//冷却率(℃/秒)
{
}

void Engine::Update(double deltaTime) {
	UpdateTorqueCurve();
	UpdateTemperature(deltaTime);
}

void Engine::SetThrottle(float throttle) {
	m_throttle = std::clamp(throttle, 0.0f, 1.0f);
}

void Engine::SetBrake(float brake) {
	m_brake = std::clamp(brake, 0.0f, 1.0f);
}

void Engine::SetRPM(float rpm) {
	m_currentRPM = std::clamp(rpm, m_idleRPM, m_maxRPM);
}

void Engine::SetMaxPower(float maxPower) {
	m_maxPower = std::max(0.0f, maxPower);
}

void Engine::SetMaxTorque(float maxTorque) {
	m_maxTorque = std::max(0.0f, maxTorque);
}

void Engine::SetMaxRPM(float maxRPM) {
	m_maxRPM = maxRPM;
}

void Engine::SetIdleRPM(float idleRPM) {
	m_idleRPM = idleRPM;
}

void Engine::SetRedlineRPM(float redlineRPM) {
	m_redlineRPM = redlineRPM;
}

float Engine::GetEngineVolume() const {
	return (m_currentRPM / m_maxRPM) * 0.8f + m_throttle * 0.2f; //回転数とスロットルに基づく音量
}

float Engine::GetEnginePitch() const {
	return 0.5f + (m_currentRPM / m_maxRPM) * 1.5f; //回転数に基づく音程
}

void Engine::UpdateTorqueCurve() {
	//リアルなエンジントルクカーブを計算
	float rpmRatio = m_currentRPM / m_maxRPM;

	//トルクカーブ(ピークは3000~4000rpmあたり)
	float torqueMultiplier = 0.0f;
	if (rpmRatio < 0.3f) {
		torqueMultiplier = 0.6f + (rpmRatio / 0.3f) * 0.4f; //0.6f～1.0f
	} else if (rpmRatio < 0.6f) {
		torqueMultiplier = 1.0f; //ピーク
	} else {
		torqueMultiplier = 1.0f - (rpmRatio - 0.6f) * 0.7f; //1.0f～0.3f
	}

	m_currentTorque = m_maxTorque * torqueMultiplier * m_throttle;
	m_currentPower = (m_currentTorque * m_currentRPM * 2.0f * 3.14159f) / (60.0f * 745.7f); //馬力に変換
}

void Engine::UpdateTemperature(double deltaTime) {
	//エンジン負荷による温度上昇
	float heatGeneration = m_throttle * m_currentRPM / m_maxRPM * 2.0f; //負荷に応じた発熱量

	//冷却による温度低下
	float cooling = m_coolingRate * (m_temperature - 20.0f); //外気温20度として冷却計算

	m_temperature += (heatGeneration - cooling) * static_cast<float>(deltaTime);
	m_temperature = std::clamp(m_temperature, 20.0f, 150.0f); //温度範囲を制限
}
