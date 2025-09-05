#pragma once

#include "gameObject.h"

//ホイール情報構造体
struct WheelInfo {
	Vector3 localPosition;	//ホイールのローカル位置
	Vector3 worldPosition;	//ホイールのワールド位置
	Vector3 rotation;		//ホイールの回転角度
	float suspensionLength;	//サスペンションの伸縮長
	bool isGrounded;		//地面に接地しているか
	float grip;				//グリップ力
	float slipRatio;		//スリップ比
	Vector3 velocity;		//ホイールの速度
	Vector3 forwardDir;		//ホイールの前方向ベクトル
	Vector3 rightDir;		//ホイールの右方向ベクトル
};

//ビークルパラメータ構造体
struct VehicleParams {
	//エンジン関連
	float maxEngineForce = 30000.0f;	//最大エンジン力 (N)
	float maxBrakingForce = 12000.0f;	//最大ブレーキ力 (N)
	float maxSteeringAngle = 0.5f;		//最大ステアリング角度(rad)

	//車体関連
	float mass = 1200.0f;				//車体質量 (kg)
	float dragCoefficient = 0.01f;		//空気抵抗係数
	float rollingResistance = 0.005f;	//転がり抵抗係数
	float centerOfMassHeight = 0.5f;	//重心高さ (m)

	//ロール関連
	float rollStiffness = 50000.0f;		//ロール剛性(Nm/rad)
	float rollDamping = 5000.0f;		//ロール減衰(Nm*s/rad)
	float maxRollAngle = 0.4f;			//最大ロール角度 (度)

	//タイヤ関連
	float maxTireGrip = 1.2f;			//最大タイヤグリップ
	float minTireGrip = 0.3f;			//最小タイヤグリップ
	float slipThreshold = 0.3f;			//スリップ開始閾値
	float driftThreshold = 0.6f;		//ドリフト開始閾値

	//サスペンション関連
	float suspensionStiffness = 80000.0f;//サスペンション剛性 (N/m)
	float suspensionDamping = 4000.0f;	//サスペンション減衰 (N*s/m)
	float maxSuspensionTravel = 0.3f;	//最大サスペンション伸縮長 (m)

	//ハンドブレーキ関連
	float handbrakeGripReduction = 0.7f; //ハンドブレーキ時のグリップ低下率
};

class Vehicle : public GameObject {
public:
	Vehicle() = default;
	~Vehicle() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;

	//車両制御インターフェース
	void SetEngineForce(float force);
	void SetBrakingForce(float brake);
	void SetSteeringValue(float steering);
	void SetHandbrake(bool active);

	//状態取得
	[[nodiscard]] float GetCurrentSpeed() const;
	[[nodiscard]] float GetMaxSpeed() const;
	[[nodiscard]] bool IsDrifting() const;
	[[nodiscard]] float GetRollAngle() const { return m_rollAngle; } //ラジアン

	//タイヤ情報取得
	Vector3 GetWheelPosition(int wheelIndex) const;
	Vector3 GetWheelRotation(int wheelIndex) const;
	float GetWheelSlipRatio(int wheelIndex) const;

protected:
	//車両パラメータ
	VehicleParams m_params;

	//車両物理状態
	Vector3 m_velocity = Vector3::ZERO; //車体の速度
	Vector3 m_angularVelocity = Vector3::ZERO; //車体の角速度
	Vector3 m_acceleration = Vector3::ZERO; //車体の加速度

	//ロール関連
	float m_rollAngle = 0.0f; //車体のロール角度 (ラジアン)
	float m_rollVelocity = 0.0f; //車体のロール角速度 (ラジアン/秒)

	//入力値
	float m_currentEngineForce = 0.0f; //現在のエンジン力
	float m_currentBrakingForce = 0.0f; //現在のブレーキ力
	float m_currentSteeringAngle = 0.0f; //現在のステアリング角度
	bool m_handbrakeActive = false; //ハンドブレーキ状態

	//タイヤ情報
	WheelInfo m_wheels[4]; //4輪分のホイール情報

	//物理計算用
	Vector3 m_centerOfMass = Vector3::ZERO; //重心オフセット
	float m_wheelBase = 2.8f; //ホイールベース (前後輪間距離)
	float m_trackWidth = 1.5f; //トレッド幅 (左右輪間距離)

private:
	//物理演算更新
	void UpdatePhysics(double deltaTime);
	void UpdateWheels(double deltaTime);
	void CalculateForces(double deltaTime);
	void ApplyForces(double deltaTime);
	void UpdateTireGrip();
	void CalculateSuspension();
	void UpdateRollPhysics(double deltaTime);

	//タイヤ物理
	Vector3 CalculateTireForce(int wheelIndex, const Vector3& wheelVelocity);
	float CalculateSlipRatio(const Vector3& wheelVelocity, const Vector3& wheelForward);
	float CalculateSlipAngle(const Vector3& wheelVelocity, const Vector3& wheelForward, const Vector3& wheelRight);
	float GetGripMultiplier(float slipRatio, float slipAngle, bool handbrakeActive);

	//車両状態更新
	void UpdateWheelPosition();
	void UpdateWheelRotations(double deltaTime);

	//抵抗力計算
	Vector3 CalculateAirResistance() const;
	Vector3 CalculateRollingResistance() const;

	//ロール計算
	float CalculateLateralAcceleration() const;
	float CalculateRollMoment() const;

	//内部状態
	float m_wheelRotationAngle[4] = { 0.0f }; //ホイールの回転角度 (ラジアン)
	bool m_isReversing = false; //後退中フラグ
};
