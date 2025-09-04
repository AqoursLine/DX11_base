#pragma once

#include "gameObject.h"

enum class VehicleState {
	GRIP_DRIVING, //グリップ走行
	DRIFT_INITIATE, //ドリフト開始
	DRIFT_ACTIVE, //ドリフト中
	DRIFT_RECOVERY //ドリフト回復
};

//車両パラメータ
struct VehicleParams {
	//エンジン設定
	float maxEngineForce = 4500.0f; //エンジンの最大出力
	float maxBrakingForce = 100.0f; //ブレーキの最大出力
	float maxSteeringAngle = 0.5f; //ハンドルの最大回転角(rad)
	float maxSpeed = 200.0f; //最大速度(km/h)
	//車体設定
	float chassisMass = 1000.0f; //車体の質量
	Vector3 chassisSize = { 2.0f, 0.6f, 4.0f }; //車体のサイズ

	//物理設定
	float acceleration = 18.0f; //加速度
	float deceleration = 6.0f; //減速度
	float friction = 0.992f; //摩擦係数
	float airResistance = 0.008f; //空気抵抗
	float steeringSensitivity = 2.2f; //ステアリング感度

	//後退設定
	float reverseForceRatio = 0.7f; //後退時エンジン力割合

	//ドリフト設定
	float driftThreshold = 30.0f; //ドリフト開始速度閾値(km/h)
	float driftInitiateForce = 3.0f; //ドリフト開始のための横方向力
	float driftSustainForce = 1.5f; //ドリフト維持のための横方向力
	float rearSlipMuktiplier = 2.5f; //後輪スリップ倍率
	float frontGripStrength = 0.95f; //前輪グリップ力
	float rearGripLoss = 0.7f; //後輪グリップ喪失率
	float driftRecoveryRate = 3.0f; //ドリフト回復速度

	//ホイール設定
	float wheelRadius = 0.5f; //ホイールの半径
	Vector3 frontLeftWheelPos = { -1.0f, -0.3f, 1.5f }; //左前ホイール位置
	Vector3 frontRightWheelPos = { 1.0f, -0.3f, 1.5f }; //右前ホイール位置
	Vector3 rearLeftWheelPos = { -1.0f, -0.3f, -1.5f }; //左後ホイール位置
	Vector3 rearRightWheelPos = { 1.0f, -0.3f, -1.5f }; //右後ホイール位置
};

class Vehicle : public GameObject {
public:
	Vehicle();
	virtual ~Vehicle() = default;

	//GameObject継承関数
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;

	//ビークル操作
	virtual void SetEngineForce(float force); //エンジン力設定
	virtual void SetSteeringValue(float steering); //ステアリング設定
	virtual void SetBrakingForce(float brake); //ブレーキ力設定
	virtual void SetHandbrake(bool handbrake); //ハンドブレーキ設定

	//ビークル状態取得
	[[nodiscard]] float GetCurrentSpeed() const { return m_currentSpeed; }
	[[nodiscard]] float GetMaxSpeed() const { return m_params.maxSpeed; }
	[[nodiscard]] float GetEngineForce() const { return m_currentEngineForce; }
	[[nodiscard]] float GetSteeringAngle() const { return m_currentSteering; }
	[[nodiscard]] float GetBrakingForce() const { return m_currentBrakingForce; }
	[[nodiscard]] bool IsReversing() const { return m_isReversing; }
	[[nodiscard]] bool IsHandbrakeActive() const { return m_handbrakeActive; }
	[[nodiscard]] bool IsDrifting() const { return m_vehicleState != VehicleState::GRIP_DRIVING; }
	[[nodiscard]] VehicleState GetVehicleState() const { return m_vehicleState; }
	[[nodiscard]] float GetDriftAngle() const { return m_driftAngle; }
	[[nodiscard]] float GetSripRatio() const {return m_slipRatio; }
	[[nodiscard]] float GetRearSlipFactor() const { return m_rearSlipFactor; }

	//ホイール状態取得
	[[nodiscard]] Vector3 GetWheelPosition(int wheelIndex) const;
	[[nodiscard]] Vector3 GetWheelRotation(int wheelIndex) const;
	[[nodiscard]] int GetNumWheels() const { return 4; }

protected:
	//ビークルパラメータ
	VehicleParams m_params;

	//現在の操作値
	float m_currentEngineForce = 0.0f; //現在のエンジン力
	float m_currentSteering = 0.0f; //現在のステアリング角
	float m_currentBrakingForce = 0.0f; //現在のブレーキ力
	bool m_handbrakeActive = false; //ハンドブレーキ状態

	//物理状態
	float m_currentSpeed = 0.0f; //現在の速度(km/h)
	Vector3 m_velocity = {0.0f, 0.0f, 0.0f}; //現在の速度ベクトル
	Vector3 m_acceleration = { 0.0f, 0.0f, 0.0f }; //現在の加速度ベクトル
	float m_angularVelocity = 0.0f; //Y軸周りの角速度(rad/s)
	bool m_isReversing = false; //後退中フラグ

	//ドリフト状態
	VehicleState m_vehicleState = VehicleState::GRIP_DRIVING; //現在のビークル状態
	float m_driftAngle = 0.0f; //ドリフト角(rad)
	float m_slipRatio = 0.0f; //スリップ比
	float m_driftIntensity = 0.0f; //ドリフト強度
	float m_rearSlipFactor = 0.0f; //後輪スリップ係数
	float m_driftTimer = 0.0f; //ドリフト時間

	//ホイール状態
	float m_wheelRotationAngle = 0.0f; //ホイールの回転角
	float m_frontWheelSteeringAngle = 0.0f; //前輪のステアリング角

	enum WheelIndex {
		FRONT_LEFT = 0,
		FRONT_RIGHT = 1,
		REAR_LEFT = 2,
		REAR_RIGHT = 3,
		WHEEL_COUNT = 4
	};
private:
	//物理計算
	void UpdatePhysics(double deltaTime);
	void ApplyEngineForce(float deltaTime);
	void ApplySteering(float deltaTime);
	void ApplyFriction(float deltaTime);
	void ApplyAirResistance(float deltaTime);
	void UpdateWheelRotation(float deltaTime);

	//ステート管理
	void UpdateVehicleState(float deltaTime);
	void HandleGripDriving(float deltaTime);
	void HandleDriftInitiate(float deltaTime);
	void HandleDriftActive(float deltaTime);
	void HandleDriftRecovery(float deltaTime);

	//ドリフト物理
	void CalculateLateralForce(float deltaTime);
	void ApplyHandbrakeDrift(float deltaTime);
	void CalculateDriftAngle();

	//ホイールラッピング
	float WrapAngle(float angle) const {
		while (angle > XM_PI) angle -= XM_2PI;
		while (angle <= -XM_PI) angle += XM_2PI;
		return angle;
	}
};

