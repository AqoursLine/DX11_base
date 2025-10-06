#pragma once
#include <vector>

//トルクカーブのデータポイント
struct TorquePoint {
	float rpm;		//回転数
	float torque;	//トルク(Nm)
};

class Engine {
public:
	Engine();
	~Engine() = default;

	//エンジン制御
	void SetThrottle(float throttle);	//スロットル設定 (0.0f ~ 1.0f)
	void SetEngineLoad(float velocity, float resistance); //負荷設定
	void Update(double deltaTime);		//更新

	//エンジン状態取得
	float GetCurrentPower() const { return m_currentPower; }	//現在出力取得
	float GetMaxPower() const { return m_maxPower; }			//最大出力取得
	float GetRPM() const { return m_rpm; }						//回転数取得
	float GetTorque() const { return m_currentTorque; }			//トルク取得
	bool IsRunning() const { return m_isRunning; }				//エンジン稼働状態取得

	//エンジンパラメータ設定
	void SetMaxPower(float maxPower) { m_maxPower = maxPower; }					//最大出力設定
	void SetAcceleration(float acceleration) { m_rpmAcceleration = acceleration; }	//加速度設定
	void SetDeceleration(float deceleration) { m_rpmDeceleration = deceleration; }	//減速度設定
	void SetMinRPM(float minRPM) { m_minRPM = minRPM; }							//アイドリング回転数設定
	void SetMaxRPM(float maxRPM) { m_maxRPM = maxRPM; }							//最大回転数設定
	void SetLoadSensitivity(float sensitivity) { m_loadSensitivity = sensitivity; }	//負荷感度設定

	//エンジン制御
	void Start() { m_isRunning = true; }	//エンジン始動
	void Stop() { m_isRunning = false; }	//エンジン停止

	//サウンド用情報
	float GetEngineLoadFactor() const;	//エンジン負荷率取得 (0.0f ~ 1.0f)

	//トルクカーブ設定
	void SetTorqueCurve(const std::vector<TorquePoint>& curve) { m_torqueCurve = curve; }

private:
	//エンジンパラメータ
	float m_maxPower;		//最大出力
	float m_currentPower;	//現在出力
	float m_currentTorque;	//現在トルク(Nm)
	float m_throttle;		//スロットル入力 (0.0f ~ 1.0f)

	float m_rpm;			//回転数
	float m_targetRPM;		//目標回転数
	float m_minRPM;			//アイドリング回転数
	float m_maxRPM;			//最大回転数

	float m_rpmAcceleration;	//加速度
	float m_rpmDeceleration;	//減速度

	float m_engineLoad;			//エンジン負荷
	float m_loadSensitivity;	//負荷感度

	bool m_isRunning;		//エンジン稼働フラグ

	//トルクカーブ (回転数とトルクの関係)
	std::vector<TorquePoint> m_torqueCurve;

	//内部計算用
	void UpdateRPM(float deltaTime);
	void UpdatePowerFromRPM();
	float GetTorqueAtRPM(float rpm) const;
	float CalculatePower(float torque, float rpm) const;
};
