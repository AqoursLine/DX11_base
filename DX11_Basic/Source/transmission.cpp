#include "transmission.h"

Transmission::Transmission()
	: m_finalDriveRatio(3.5f)
	, m_reverseRatio(-3.0f)
	, m_currentGear(1)
	, m_clutchEngagement(1.0f)
	, m_isAutomatic(true)
	, m_isShifting(false)
	, m_shiftUpRPM(6000.0f)
	, m_shiftDownRPM(2000.0f)
	, m_shiftTimer(0.0)
	, m_shiftDuration(0.5f)
{
	//デフォルトのギア比設定
	m_gearRatios = { 3.5f, 2.1f, 1.5f, 1.1f, 0.9f, 0.7f };
}

void Transmission::Update(double deltaTime, float engineRPM, float vehicleSpeed) {
	if (m_isAutomatic) {
		UpdateAutomaticShifting(engineRPM, vehicleSpeed);
	}
	UpdateClutch(deltaTime);
}

void Transmission::ShiftUp() {
	if (m_currentGear < static_cast<int>(m_gearRatios.size()) && !m_isShifting) {
		m_currentGear++;
		m_isShifting = true;
		m_shiftTimer = 0.0;
		m_clutchEngagement = 0.0f; //シフト中はクラッチを切る
	}
}

void Transmission::ShiftDown() {
	if (m_currentGear > 1 && !m_isShifting) {
		m_currentGear--;
		m_isShifting = true;
		m_shiftTimer = 0.0;
		m_clutchEngagement = 0.0f; //シフト中はクラッチを切る
	}
}

void Transmission::SetGear(int gear) {
	if (gear >= -1 && gear <= static_cast<int>(m_gearRatios.size()) && !m_isShifting) {
		m_currentGear = gear;
		m_isShifting = true;
		m_shiftTimer = 0.0;
		m_clutchEngagement = 0.0f; //シフト中はクラッチを切る
	}
}

float Transmission::GetCurrentGearRatio() const {
	if (m_currentGear == -1) {
		return m_reverseRatio;
	} else if (m_currentGear == 0) {
		return 0.0f; //ニュートラル
	} else if (m_currentGear <= static_cast<int>(m_gearRatios.size())) {
		return m_gearRatios[m_currentGear - 1];
	}
	return 1.0f;
}

float Transmission::GetOutputTorque(float inputTorque) const {
	float gearRatio = GetCurrentGearRatio();
	return inputTorque * gearRatio * m_finalDriveRatio * m_clutchEngagement;
}

void Transmission::UpdateAutomaticShifting(float engineRPM, float vehicleSpeed) {
	if (m_isShifting) {
		return; //シフト中は自動シフトしない
	}

	//シフトアップ判定
	if (engineRPM > m_shiftUpRPM && m_currentGear < static_cast<int>(m_gearRatios.size())) {
		ShiftUp();
	}

	//シフトダウン判定
	if (engineRPM < m_shiftDownRPM && m_currentGear > 1 && vehicleSpeed > 5.0f) {
		ShiftDown();
	}
}

void Transmission::UpdateClutch(double deltaTime) {
	if (m_isShifting) {
		m_shiftTimer += deltaTime;

		if (m_shiftTimer < m_shiftDuration * 0.3f) {
			//シフト開始から30%の時間はクラッチを切る
			m_clutchEngagement = 0.0f;
		} else if (m_shiftTimer < m_shiftDuration) {
			//クラッチをつなぐ
			float progress = (m_shiftTimer - m_shiftDuration * 0.3f) / (m_shiftDuration * 0.7f);
			m_clutchEngagement = progress;
		} else {
			//シフト完了
			m_clutchEngagement = 1.0f;
			m_isShifting = false;
		}
	}
}
