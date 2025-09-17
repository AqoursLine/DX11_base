#pragma once

//==================================================================
// エンジンクラス
//==================================================================

class Engine {
public:
	Engine();
	~Engine() = default;

	void Update(double deltaTime);

	//エンジン制御
	void SetThrottle(float throttle);	//0.0f～1.0f
	void SetBrake(float brake);		//0.0f～1.0f
	void SetRPM(float rpm);

	//エンジン特性設定
	void SetMaxPower(float maxPower);	//最大出力(馬力)
	void SetMaxTorque(float maxTorque);	//最大トルク(Nm)
	void SetMaxRPM(float maxRPM);		//最大回転数(rpm)
	void SetIdleRPM(float idleRPM);		//アイドリング回転数(rpm)
	void SetRedlineRPM(float redlineRPM);	//レッドライン回転数(rpm)

	//取得
	float GetCurrentPower() const { return m_currentPower; }		//現在の出力(馬力)
	float GetCurrentTorque() const { return m_currentTorque; }	//現在のトルク(kgf･m)
	float GetRPM() const { return m_currentRPM; }			//現在の回転数(rpm)
	float GetThrottle() const { return m_throttle; }		//スロットル開度(0.0f～1.0f)
	float GetBrake() const { return m_brake; }			//ブレーキ圧(0.0f～1.0f)
	float GetMaxRPM() const { return m_maxRPM; }		//最大回転数(rpm)
	float GetRedlineRPM() const { return m_redlineRPM; }	//レッドライン回転数(rpm)
	float GerTemperature() const { return m_temperature; }	//現在の温度(℃)
	bool IsOverheated() const { return m_temperature > m_maxTemperature; }

	//エンジン音用
	float GetEngineVolume() const;	//エンジン音量(0.0f～1.0f)
	float GetEnginePitch() const;	//エンジン音程(0.5f～2.0f)

private:
	void UpdateTorqueCurve();
	void UpdateTemperature(double deltaTime);

	//エンジン特性
	float m_maxPower;	//最大出力(馬力)
	float m_maxTorque;	//最大トルク(kgf･m)
	float m_maxRPM;		//最大回転数(rpm)
	float m_idleRPM;	//アイドリング回転数(rpm)
	float m_redlineRPM;	//レッドライン回転数(rpm)

	//現在の状態
	float m_currentRPM;		//現在の回転数(rpm)
	float m_currentPower;	//現在の出力(馬力)
	float m_currentTorque;	//現在のトルク(kgf･m)
	float m_throttle;		//スロットル開度(0.0f～1.0f)
	float m_brake;			//ブレーキ圧(0.0f～1.0f)

	//温度管理
	float m_temperature;	//現在の温度(℃)
	float m_maxTemperature;	//最大許容温度(℃)
	float m_coolingRate;	//冷却率(℃/秒)

};
