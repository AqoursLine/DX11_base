#pragma once

#include "gameObject.h"
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <bullet/BulletDynamics/Vehicle/btVehicleRaycaster.h>


struct VehicleParams {
	//エンジン設定
	float maxEngineForce = 2000.0f; //エンジンの最大出力
	float maxBreakingForce = 100.0f; //ブレーキの最大出力
	float engineForce = 0.0f; //エンジンの出力

	//車体設定
	float chassisMass = 800.0f; //車体の質量
	Vector3 chassisSize = { 2.0f, 0.6f, 4.0f }; //車体のサイズ
	Vector3 localInertia = { 0.0f, 0.0f, 0.0f }; //車体の慣性モーメント

	//ホイール設定
	float wheelRadius = 0.5f; //ホイールの半径
	float wheelWidth = 0.4f; //ホイールの幅
	float wheelFriction = 1000.0f; //ホイールの摩擦係数
	float wheelDamping = 0.2f; //ホイールのダンピング
	float wheelCompression = 0.84f; //ホイールの圧縮率
	float suspensionStiffness = 20.0f; //サスペンションの硬さ
	float suspensionRestLength = 0.6f; //サスペンションの伸び縮みの長さ
	float rollInfluence = 0.1f; //ロールの影響度

	//ホイール接続点(ローカル座標)
	Vector3 frontLeftWheelPos = { -1.0f, -0.3f, 1.5f };
	Vector3 frontRightWheelPos = { 1.0f, -0.3f, 1.5f };
	Vector3 rearLeftWheelPos = { -1.0f, -0.3f, -1.5f };
	Vector3 rearRightWheelPos = { 1.0f, -0.3f, -1.5f };
};

class Vehicle : public GameObject {
public:
	Vehicle(btDiscreteDynamicsWorld* world, const VehicleParams& params = VehicleParams());
	virtual ~Vehicle();

	//GameObject継承メソッド
	virtual bool Initialize() override;
	virtual void Update(double deltaTime) override;
	virtual void Finalize() override;

	//ビークル操作
	virtual void SetEngineForce(float force); //エンジン出力設定
	virtual void SetSteeringValue(float value); //ハンドル操作
	virtual void SetBrakingForce(float force); //ブレーキ力設定

	//ビークル状態取得
	float GetCurrentSpeed() const; //現在の速度取得
	float GetEngineForce() const { return m_currentEngineForce; }
	float GetSteeringValue() const { return m_currentSteering; }
	float GetBrekingForce() const {	return m_currentBrakingForce; }

protected:
	//現在の操作値
	float m_currentEngineForce = 0.0f; //現在のエンジン出力
	float m_currentSteering = 0.0f; //現在のハンドル操作
	float m_currentBrakingForce = 0.0f; //現在のブレーキ力
};
