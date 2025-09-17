#pragma once

#include <vector>
#include <memory>

//前方宣言
class Wheel;

//==================================================================
// ブレーキシステムクラス
//==================================================================

class BrakeSystem {
public:
	BrakeSystem();
	~BrakeSystem() = default;

	void Update(double deltaTime);

	//ブレーキ操作
	void SetBrakeInput(float input) { m_brakeInput = input; }
	void SetHandbrake(bool engaged) { m_handbrakeEngaged = engaged; }

	//ブレーキ特性設定
	void SetMaxBrakeForce(float force) { m_maxBrakeForce = force; } //最大ブレーキ力(N)
	void SetFrontBrakeBias(float bias) { m_frontBrakeBias = bias; } //前後ブレーキ力配分(0.0f～1.0f)
	void SetABS(bool enabled) { m_absEnabled = enabled; } //ABSの有効/無効

	//ABS制御
	void UpdateABS(const std::vector<std::shared_ptr<Wheel>>& wheels);

	//取得
	float GetBrakeForce() const;
	float GetRearBrakeForce() const;
	float GetHandbrakeForce() const { return m_handbrakeEngaged ? m_maxBrakeForce * 0.8f : 0.0f; }
	bool IsABSActive() const { return m_absActive; }

private:
	float m_brakeInput; //ブレーキ入力(0.0f～1.0f)
	float m_maxBrakeForce; //最大ブレーキ力(N)
	float m_frontBrakeBias; //前後ブレーキ力配分(0.0f～1.0f)
	bool m_handbrakeEngaged; //サイドブレーキがかかっているかどうか

	//ABS
	bool m_absEnabled; //ABSが有効かどうか
	bool m_absActive; //ABSが作動中かどうか
	float m_absThreshold; //ABS作動スリップ率閾値
	std::vector<bool> m_wheelAbs; //各ホイールのABS作動状態
};
