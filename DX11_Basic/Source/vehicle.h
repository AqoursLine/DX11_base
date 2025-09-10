#pragma once

#include "gameObject.h"
#include <algorithm>


struct Wheel {
	Vector3 localPosition;		//ローカル位置
	Vector3 worldPosition;		//ワールド位置
	Vector3 suspensionForce;	//サスペンション力
	Vector3 tireForce;			//タイヤ力

	float radius;				//タイヤ半径
	float width;				//タイヤ幅
	float steerAngle;			//ステアリング角
	float rotationAngle;		//回転角
	float angularVelocity;		//角速度

	bool isGrounded;			//接地しているか
	bool isFrontWheel;			//前輪か
	bool isDriveWheel;			//駆動輪か
	float compressionRatio;			//サスペンションの圧縮率

	//タイヤ物理パラメータ
	float maxGripForce;			//最大グリップ力
	float frictionCoefficient;	//摩擦係数
	float slipRatio;			//スリップ率
	float slipAngle;			//スリップ角

	Wheel()
		: localPosition(Vector3::ZERO)
		, worldPosition(Vector3::ZERO)
		, suspensionForce(Vector3::ZERO)
		, tireForce(Vector3::ZERO)
		, radius(0.3f)
		, width(0.2f)
		, steerAngle(0.0f)
		, rotationAngle(0.0f)
		, angularVelocity(0.0f)
		, isGrounded(false)
		, isFrontWheel(false)
		, isDriveWheel(false)
		, compressionRatio(0.0f)
		, maxGripForce(2000.0f)
		, frictionCoefficient(0.8f)
		, slipRatio(0.0f)
		, slipAngle(0.0f)
	{
	}
};

struct Suspension {
	float springConstant;	//バネ定数
	float damperConstant;	//ダンパー定数
	float restLength;		//自然長
	float maxCompression;	//最大圧縮量
	float currentLength;	//現在の長さ
	float lastLength;		//前フレームの長さ
	Vector3 compressionVelocity; //圧縮速度

	Suspension()
		: springConstant(15000.0f)
		, damperConstant(1500.0f)
		, restLength(0.35f)
		, maxCompression(0.25f)
		, currentLength(0.4f)
		, lastLength(0.4f)
		, compressionVelocity(Vector3::ZERO)
	{
	}
};

struct Engine {
	float maxTorque;			//最大トルク
	float maxRPM;				//最大回転数
	float currentRPM;			//現在の回転数
	float throttleInput;			//スロットル入力
	float idleRPM;				//アイドル回転数
	float engineBraking;		//エンジンブレーキ係数

	Engine()
		: maxTorque(800.0f)
		, maxRPM(7000.0f)
		, currentRPM(800.0f)
		, throttleInput(0.0f)
		, idleRPM(800.0f)
		, engineBraking(50.0f)
	{
	}

	//トルクカーブの計算
	float GetTorque() const {
		if (currentRPM <= 0) return 0.0f;

		//シンプルなトルクカーブ
		float normalizedRPM = currentRPM / maxRPM;
		float torqueMultiplier;
		
		if (normalizedRPM < 0.3f) {
			//低回転域
			torqueMultiplier = 0.5f + (normalizedRPM / 0.3f) * 0.5f;
		} else if (normalizedRPM < 0.7f) {
			//最適回転域
			torqueMultiplier = 1.0f;
		} else {
			//高回転域
			torqueMultiplier = 1.0f - ((normalizedRPM - 0.7f) / 0.3f) * 0.4f;
		}

		return maxTorque * torqueMultiplier * throttleInput;
	}

};

class Vehicle : public GameObject {
public:
	Vehicle();
	virtual ~Vehicle() = default;

	//制御入力の設定
	void SetSteerInput(float input) { m_steerInput = std::clamp(input, -1.0f, 1.0f); }
	void SetThrottleInput(float input) { m_throttleInput = std::clamp(input, -1.0f, 1.0f); }
	void SetBrakeInput(float input) { m_brakeInput = std::clamp(input, 0.0f, 1.0f); }
	void SetHandbrakeInput(bool input) { m_handbrakeInput = input; }

	//状態取得
	Vector3 GetVelocity() const { return m_velocity; }
	float GetSpeed() const { return m_velocity.Length(); }
	float GetSpeedKmh() const { return GetSpeed() * 3.6f; } // m/s to km/h
	float GetRPM() const { return m_engine.currentRPM; }
	Vector3 GetAngularVelocity() const { return m_angularVelocity; }

	//車両パラメータ取得
	float GetMass() const { return m_mass; }
	const std::vector<Wheel>& GetWheels() const { return m_wheels; }
	const Engine& GetEngine() const { return m_engine; }

	//タイヤ描画用の情報取得
	struct WheelRenderInfo {
		Vector3 position;		//ワールド位置
		Vector3 rotation;		//回転角
		float compressionRatio;	//サスペンション圧縮率 (0.0f ~ 1.0f)
		bool isGrounded;		//接地しているか
		float radius;			//タイヤ半径
		float width;			//タイヤ幅
		bool isFrontWheel;		//前輪か
		bool isDrivenWheel;		//駆動輪か
	};

	//指定したインデックスのタイヤ情報を取得
	WheelRenderInfo GetWheelRenderInfo(int wheelIndex) const;

	//全タイヤの情報を取得
	std::vector<WheelRenderInfo> GetAllWheelRenderInfo() const;

protected:
	//物理プロパティ
	Vector3 m_velocity;			//速度
	Vector3 m_angularVelocity;	//角速度
	Vector3 m_acceleration;		//加速度
	Vector3 m_centerOfMass;		//重心位置

	float m_mass;				//質量
	Vector3 m_inertiaTensor;	//慣性テンソル

	//車両コンポーネント
	std::vector<Wheel> m_wheels;			//タイヤ
	std::vector<Suspension> m_suspensions; //サスペンション
	Engine m_engine;						//エンジン

	//制御入力
	float m_steerInput;		//ステアリング入力 (-1.0f ~ 1.0f)
	float m_throttleInput;	//スロットル入力 (-1.0f ~ 1.0f)
	float m_brakeInput;		//ブレーキ入力 (0.0f ~ 1.0f)
	bool m_handbrakeInput;	//ハンドブレーキ

	//車両パラメータ
	float m_wheelBase;			//ホイールベース
	float m_trackWidth;			//トレッド幅
	float m_maxSteerAngle;		//最大ステアリング角

	//空気抵抗
	float m_dragCoefficient;	//空気抵抗係数
	float m_frontalArea;		//正面投影面積

	//地面接触
	float m_groundHeight;		//地面の高さ

	bool Initialize() override;
	void Update(double deltaTime) override;

private:
	//物理計算
	void UpdatePhysics(float deltaTime);
	void UpdateEngine(float deltaTime);
	void CalculateSuspensionForces(float deltaTime);
	void CalculateWheelForces(float deltTime);
	void UpdateAllWheelRotations(float deltaTime);
	void CalculateAerodynamicsForces();
	void IntegrateForces(float deltaTime);
	void UpdateWheelPositions();

	//タイヤ物理
	Vector3 CalculateTireForce(const Wheel& wheel, const Vector3& wheelVelocity);
	float CalculateSlipRatio(const Wheel& wheel, const Vector3& wheelVelocity);
	float CalculateSlipAngle(const Wheel& wheel, const Vector3& wheelVelocity);
	void UpdateWheelRotation(int wheelIndex, const Vector3& wheelVelocity, float deltaTime);

	void ApplyInertiaSteeringForce(Wheel& wheel, const Vector3& wheelVelocity, Vector3& tireForce);
	void ApplyDirectionTrackingForce();
	void CalculateAckermannSteering();

	//地面との衝突判定
	float GetGroundHeight(const Vector3& position) const;

	//座標変換ヘルパー
	Vector3 LocalToWorld(const Vector3& localPos) const;
	Vector3 WorldToLocal(const Vector3& worldPos) const;

};
