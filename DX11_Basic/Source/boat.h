#pragma once

#include "gameObject.h"
#include "engine.h"

class Water;

class Boat : public GameObject {
public:
	Boat();
	virtual ~Boat() = default;

	//ボート制御
	void SetThrottle(float throttle);	//スロットル設定 (0.0f ~ 1.0f)
	void SetSteering(float steering);	//ステアリング設定 (-1.0f ~ 1.0f)
	void SetBrake(float brake);			//ブレーキ設定 (0.0f ~ 1.0f)
	void SetReverse(bool isReverse);	//リバースギア設定

	//ギア状態
	bool IsReversing() const { return m_isReverse; } //リバースギア状態取得

	//物理パラメータ設定
	void SetMass(float mass) { m_mass = mass; }
	void SetDrag(float drag) { m_waterDrag = drag; }
	void SetTurnRate(float turnRate) { m_maxTurnRate = turnRate; }
	void SetMaxSpeed(float maxSpeed) { m_maxSpeed = maxSpeed; }
	void SetMaxReverseSpeed(float maxReverseSpeed) { m_maxReverseSpeed = maxReverseSpeed; }
	void SetRollAmount(float rollAmount) { m_rollAmount = rollAmount; }

	//ボートの状態取得
	Vector3 GetVelocity() const { return m_velocity; }			//速度ベクトル取得
	Vector3 GetAcceleration() const { return m_acceleration; }	//加速度ベクトル取得
	float GetSpeed() const { return m_velocity.Length(); }		//速度取得
	float GetSpeedKmh() const { return GetSpeed() * 3.6f; }		//速度(km/h)取得
	Engine& GetEngine() { return m_engine; }					//エンジン参照取得
	const Engine& GetEngine() const { return m_engine; }		//エンジン参照取得(定数版)

	//水面設定
	void SetWater(Water* water) { m_water = water; }

	//ボートの寸法設定
	void SetDimensions(float length, float width, float height) {
		m_length = length;
		m_width = width;
		m_height = height;
	}

	float GetLength() const { return m_length; }
	float GetWidth() const { return m_width; }
	float GetHeight() const { return m_height; }

protected:
	virtual bool Initialize() override;
	virtual void Update(double deltaTime) override;

private:
	//物理計算
	void UpdatePhysics(float deltaTime);
	Vector3 CalculateThrustForce() const;
	Vector3 CalculateLateralForce() const;
	Vector3 CalculateDragForce() const;
	Vector3 CalculateBuoyancyForce() const;
	Vector3 CalculateWaveForce();
	void UpdateWaterInteraction(float deltaTime);
	void ApplyForces(float deltaTime);

	//姿勢制御
	void UpdateRotation(float deltaTime);
	void UpdateYaw(float deltaTime);
	Vector4 UpdateRoll(float deltaTime);
	Vector4 UpdatePitch(float deltaTime);
	void UpdateBobbing(float deltaTime);

	//エンジン
	Engine m_engine;

	//制御入力
	float m_throttleInput;	// スロットル入力 (0.0f ~ 1.0f)
	float m_steeringInput;	// ステアリング入力 (-1.0f ~ 1.0f)
	float m_brakeInput;		// ブレーキ入力 (0.0f ~ 1.0f)
	bool m_isReverse;		// リバースギアフラグ

	//物理パラメータ
	float m_mass;				// 質量
	float m_waterDrag;			// 水の抵抗係数
	float m_maxTurnRate;		// 最大旋回速度(rad/s)
	float m_maxSpeed;			// 最大前進速度(m/s)
	float m_maxReverseSpeed;	// 最大後退速度(m/s)
	float m_verticalDamping;	// 垂直方向の減衰係数
	float m_buoyancyStiffness;	// 浮力の剛性係数
	float m_restingWaterLevel;	// 静止水面の高さ
	float m_waveFolllowStrength;// 波の追従距離
	Vector3 m_lastWaveForce;	// 最後に受けた波の力

	//物理状態
	Vector3 m_velocity;			// 速度ベクトル
	Vector3 m_acceleration;		// 加速度ベクトル
	Vector3 m_angularVelocity;	// 角速度

	//前フレームの位置
	Vector3 m_prevPosition;

	//水面への参照
	Water* m_water;

	//ボートの寸法
	float m_length;	// ボートの長さ
	float m_width;	// ボートの幅
	float m_height;	// ボートの高さ

	//姿勢制御パラメータ
	float m_rollAmount;			// ロール量
	float m_pitchAmount;		// ピッチ量
	Vector4 m_yawRotation;		// 現在のヨー角クォータニオン

	//物理定数
	static constexpr float GRAVITY = 9.81f; // 重力加速度 (m/s^2)
	static constexpr float WATER_DENSITY = 1000.0f; // 水の密度 (kg/m^3)
	static constexpr float MAX_ROLL_ANGLE = 0.3f; // 最大ロール角 (ラジアン)
	static constexpr float MAX_PITCH_ANGLE = 0.2f; // 最大ピッチ角 (ラジアン)

};
