#pragma once

#include "gameObject.h"
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <bullet/BulletDynamics/Vehicle/btVehicleRaycaster.h>


struct VehicleParams {
	//エンジン設定
	float maxEngineForce = 3000.0f; //エンジンの最大出力
	float maxBrakingForce = 100.0f; //ブレーキの最大出力
	float maxSteeringAngle = 0.3f; //ハンドルの最大回転角(rad)

	float maxSpeed = 200.0f; //最大速度(km/h)

	//車体設定
	float chassisMass = 1000.0f; //車体の質量
	Vector3 chassisSize = { 2.0f, 0.6f, 4.0f }; //車体のサイズ
	Vector3 localInertia = { 0.0f, 0.0f, 0.0f }; //車体の慣性モーメント

	//ホイール設定
	float wheelRadius = 0.5f; //ホイールの半径
	float wheelWidth = 0.4f; //ホイールの幅
	float wheelFriction = 1000.0f; //ホイールの摩擦係数
	float wheelDamping = 0.8f; //ホイールのダンピング
	float wheelCompression = 0.84f; //ホイールの圧縮率
	float suspensionStiffness = 20.0f; //サスペンションの硬さ
	float suspensionRestLength = 0.6f; //サスペンションの伸び縮みの長さ
	float rollInfluence = 0.03f; //ロールの影響度

	//ホイール接続点(ローカル座標)
	Vector3 frontLeftWheelPos = { -1.0f, -0.3f, 1.5f };
	Vector3 frontRightWheelPos = { 1.0f, -0.3f, 1.5f };
	Vector3 rearLeftWheelPos = { -1.0f, -0.3f, -1.5f };
	Vector3 rearRightWheelPos = { 1.0f, -0.3f, -1.5f };
};

class Vehicle : public GameObject {
public:
	Vehicle();
	virtual ~Vehicle() = default;

	//GameObject継承メソッド
	virtual bool Initialize() override;
	virtual void Update(double deltaTime) override;
	virtual void Finalize() override;

	//ビークル操作
	virtual void SetEngineForce(float force); //エンジン出力設定
	virtual void SetSteeringValue(float steering); //ハンドル操作
	virtual void SetBrakingForce(float brake); //ブレーキ力設定

	//ビークル状態取得
	[[nodiscard]] float GetCurrentSpeed() const; //現在の速度取得
	[[nodiscard]] float GetMaxSpeed() const { return m_params.maxSpeed; } //最大速度取得
	[[nodiscard]] float GetEngineForce() const { return m_currentEngineForce; }
	[[nodiscard]] float GetSteeringValue() const { return m_currentSteering; }
	[[nodiscard]] float GetBrekingForce() const {	return m_currentBrakingForce; }

	//物理ボディ取得
	[[nodiscard]] btRigidBody* GetChassisBody() const { return m_vehicleBody; }
	[[nodiscard]] btRaycastVehicle* GetVehicle() const { return m_vehicle; }

	//ホイール情報取得
	[[nodiscard]] btTransform GetWheelTransform(int wheelIndex) const;
	[[nodiscard]] int GetNumWheels() const { return m_vehicle ? m_vehicle->getNumWheels() : 0; }

protected:
	//物理世界
	btDynamicsWorld* m_dynamicsWorld = nullptr;

	//ビークル物理オブジェクト
	btRigidBody* m_vehicleBody = nullptr; //車体の剛体
	btRaycastVehicle* m_vehicle = nullptr; //ビークル本体
	btVehicleRaycaster* m_vehicleRayCaster = nullptr; //レイキャスター
	btRaycastVehicle::btVehicleTuning m_tuning; //ビークルチューニング

	//車体形状
	btCollisionShape* m_chassisShape = nullptr; //車体の形状

	//ビークルパラメータ
	VehicleParams m_params;

	//現在の操作値
	float m_currentEngineForce = 0.0f; //現在のエンジン出力
	float m_currentSteering = 0.0f; //現在のハンドル操作
	float m_currentBrakingForce = 0.0f; //現在のブレーキ力

	//ホイールインデックス
	enum WheelIndex {
		FRONT_LEFT = 0,
		FRONT_RIGHT = 1,
		REAR_LEFT = 2,
		REAR_RIGHT = 3,
		WHEEL_COUNT = 4
	};

	[[nodiscard]] Vector3 ToVector3(const btVector3& v) const noexcept {
		return Vector3(v.getX(), v.getY(), v.getZ());
	}

	//クォータニオンをオイラー角に変換
	[[nodiscard]] Vector3 QuaternionToEuler(const btQuaternion& q);

private:
	void CreateChassis(); //車体の作成
	void AddWheels(); //ホイールの追加
	void UpdateTransform(); //物理世界からグラフィックス世界へ変換更新
	inline float WrapAngle(float angle) const {
		while (angle > XM_PI) angle -= XM_2PI;
		while (angle <= -XM_PI) angle += XM_2PI;
		return angle;
	}

	//Vector3とbtVector3の変換ヘルパー
	[[nodiscard]] btVector3 ToBtVector3(const Vector3& v) const noexcept {
		return btVector3(v.x, v.y, v.z);
	}
};
