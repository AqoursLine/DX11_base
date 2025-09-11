#pragma once

#include "gameObject.h"

//タイヤの位置を表す列挙型
enum class WheelPosition {
	FRONT_LEFT = 0,
	FRONT_RIGHT,
	REAR_LEFT,
	REAR_RIGHT,
};

//個別タイヤの状態を管理する構造体
struct WheelData {
	Vector3 position; //タイヤの相対位置
	Vector3 velocity; //タイヤの速度
	Vector3 force;    //タイヤが発生する力
	float load;    //タイヤにかかる荷重
	float grip;   //タイヤのグリップ力
	float slipRatio; //タイヤのスリップ率
	float slipAngle; //タイヤのスリップ角
	float steerAngle; //タイヤのステア角
	bool isGrounded; //タイヤが地面に接地しているか

	WheelData()
		: position(0.0f, 0.0f, 0.0f)
		, velocity(0.0f, 0.0f, 0.0f)
		, force(0.0f, 0.0f, 0.0f)
		, load(0.0f)
		, grip(1.0f)
		, slipRatio(0.0f)
		, slipAngle(0.0f)
		, steerAngle(0.0f)
		, isGrounded(true) {
	}
};

class Vehicle : public GameObject {
public:
	Vehicle();
	virtual ~Vehicle() = default;

	//車両状態取得
	float GetSpeed() const { return m_currentSpeed; } //速度(m/s)
	Vector3 GetVelocity() const { return m_velocity; } //速度ベクトル(m/s)
	float GetRPM() const { return m_engineRPM; } //エンジン回転数(rpm)

protected:
	virtual bool Initialize() override;
	virtual void Update(double deltaTime) override;
	virtual void Finalize() override;

	void SetThrottle(float throttle);
	void SetSteering(float streering);
	void SetBrake(float brake);

private:
	//入力値
	float m_throttleInput;	//アクセル入力
	float m_steeringInput;	//ハンドル入力
	float m_brakeInput;		//ブレーキ入力

	//物理パラメータ
	Vector3 m_velocity;		//速度
	Vector3 m_acceleration;	//加速度
	float m_currentSpeed;	//現在の速度(スカラ―)
	float m_mass;			//質量(kg)

	//エンジンパラメータ
	float m_engineRPM;		//エンジン回転数
	float m_maxRPM;			//最大回転数
	float m_idleRPM;		//アイドリング回転数
	float m_enginePower;	//エンジン出力(kw)

	//車両特性
	float m_maxSpeed;			//最高速度(m/s)
	float m_accelerrationForce; //加速力(N)
	float m_brakeForce;			//制動力(N)
	float m_friction;			//摩擦抵抗係数
	float m_airResistance;		//空気抵抗係数
	float m_rollingResistance;	//転がり抵抗係数

	//ステアリング特性
	float m_maxSteerAngle;		//最大ステア各(rad)
	float m_steerSpeed;			//ステアリング応答速度(rad/s)
	float m_currentSteerAngle;	//現在のステア角(rad)
	float m_lateralGrip;		//横方向グリップ係数
	float m_underSteerGradient; //アンダーステア係数

	//車両寸法
	float m_wheelBase;		//ホイールベース(m)
	float m_trackWidth;		//トレッド幅(m)
	float m_cgHeight;		//重心高(m)

	//内部状態
	bool m_isEngineRunning; //エンジン稼働状態
	float m_gearRatio;		//ギア比
	float m_angularVelocity; //車両の角速度(rad/s)
	Vector3 m_lateralVelocity; //横方向速度

	//物理計算
	void UpdateEngine(float deltaTime);
	void UpdateSteering(float deltaTime);
	void UpdatePhysics(float deltaTime);
	void UpdateMovement(float deltaTime);

	//力の計算
	Vector3 CalculateEngineForce();
	Vector3 CalculateBrakeForce();
	Vector3 CalculateFrictionForce();
	Vector3 CalculateAirResistance();
	Vector3 CalculateSteeringForce();

	//ステアリング計算
	Vector3 CalculateLateralForce() const;
	float CalculateSteerAngle() const;
	float CalculateTurnRadius() const;
};

