#pragma once

#include <vector>

//==================================================================
// トランスミッションクラス
//==================================================================

class Transmission {
public:
	Transmission();
	~Transmission() = default;

	void Update(double deltaTime, float engineRPM, float vehicleSpeed);

	//ギア操作
	void ShiftUp();
	void ShiftDown();
	void SetGear(int gear); //ギア設定(-1: R, 0: N, 1～: 前進ギア)
	void SetAutomatic(bool isAutomatic) { m_isAutomatic = isAutomatic; }

	//ギア比設定
	void SetGearRatios(const std::vector<float>& gearRatios) { m_gearRatios = gearRatios; } //各ギアの変速比
	void SetFinalDriveRatio(float ratio) { m_finalDriveRatio = ratio; } //ファイナルドライブ比
	void SetReverseRatio(float ratio) { m_reverseRatio = ratio; } //リバースギアの変速比

	//クラッチ操作
	void SetClutchEngagement(float clutch) { m_clutchEngagement = clutch; }

	//取得
	int GetCurrentGear() const { return m_currentGear; } //現在のギア(-1: R, 0: N, 1～: 前進ギア)
	float GetCurrentGearRatio() const;
	float GetOutputTorque(float inputTorque) const; //入力トルクから出力トルクを計算
	float GetClutchEngagement() const { return m_clutchEngagement; } //クラッチの噛み具合(0.0f～1.0f)
	bool IsAutomatic() const { return m_isAutomatic; }
	bool IsShifting() const { return m_isShifting; } //シフト中かどうか

private:
	void UpdateAutomaticShifting(float engineRPM, float vehicleSpeed);
	void UpdateClutch(double deltaTime);

	//ギア設定
	std::vector<float> m_gearRatios; //各ギアの変速比
	float m_finalDriveRatio; //ファイナルドライブ比
	float m_reverseRatio; //リバースギアの変速比

	//現在の状態
	int m_currentGear; //現在のギア(-1: R, 0: N, 1～: 前進ギア)
	float m_clutchEngagement; //クラッチの噛み具合(0.0f～1.0f)
	bool m_isAutomatic; //オートマチックかどうか
	bool m_isShifting; //シフト中かどうか

	//オートマチック制御
	float m_shiftUpRPM; //シフトアップ回転数(rpm)
	float m_shiftDownRPM; //シフトダウン回転数(rpm)
	double m_shiftTimer; //シフト中のタイマー(秒)
	float m_shiftDuration; //シフトにかかる時間(秒)


};
