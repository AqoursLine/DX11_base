#pragma once

class Engine {
public:
	Engine();
	~Engine() = default;

	//エンジン制御
	void SetThrottle(float throttle);	//スロットル設定 (0.0f ~ 1.0f)
	void Update(double deltaTime);		//更新

	//エンジン状態取得
	float GetCurrentPower() const { return m_currentPower; }	//現在出力取得
	float GetMaxPower() const { return m_maxPower; }			//最大出力取得
	float GetRPM() const { return m_rpm; }						//回転数取得
	bool IsRunning() const { return m_isRunning; }				//エンジン稼働状態取得

	//エンジンパラメータ設定
	void SetMaxPower(float maxPower) { m_maxPower = maxPower; }					//最大出力設定
	void SetAcceleration(float acceleration) { m_acceleration = acceleration; }	//加速度設定
	void SetDeceleration(float deceleration) { m_deceleration = deceleration; }	//減速度設定
	void SetMinRPM(float minRPM) { m_minRPM = minRPM; }							//アイドリング回転数設定
	void SetMaxRPM(float maxRPM) { m_maxRPM = maxRPM; }							//最大回転数設定

	//エンジン制御
	void Start() { m_isRunning = true; }	//エンジン始動
	void Stop() { m_isRunning = false; }	//エンジン停止

	//サウンド用情報
	float GetEngineLoadFactor() const;	//エンジン負荷率取得 (0.0f ~ 1.0f)

private:
	//エンジンパラメータ
	float m_maxPower;		//最大出力
	float m_currentPower;	//現在出力
	float m_targetPower;	//目標出力
	float m_throttle;		//スロットル入力 (0.0f ~ 1.0f)

	float m_rpm;			//回転数
	float m_minRPM;			//アイドリング回転数
	float m_maxRPM;			//最大回転数

	float m_acceleration;	//加速度
	float m_deceleration;	//減速度

	bool m_isRunning;		//エンジン稼働フラグ

	//内部計算用
	void UpdatePower(float deltaTime);
	void UpdateRPM(float deltaTime);
};
