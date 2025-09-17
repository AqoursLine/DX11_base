#pragma once

#include "gwVector.h"

//==================================================================
// ホイールクラス
//==================================================================

class Wheel {
public:
	Wheel();
	~Wheel() = default;

	void Update(double deltaTime, const Vector3& vehicleVelocity, float engineTorque, bool isBraking);

	void SetRadius(float radius) { m_radius = radius; }					//ホイール半径(m)
	void SetMass(float mass) { m_mass = mass; }
	void SetPosition(const Vector3& localPosition) { m_localPosition = localPosition; } //ホイールの車体に対するローカル位置(m)
	void SetSuspensionSettings(float stiffness, float damping, float restLength);
	void SetTireSettings(float gripFront, float gripSide, float friction);
	void SetSteerAngle(float angle) { m_steerAngle = angle; } //操舵角(rad)
	void SetIsDriveWheel(bool isDrive) { m_isDriveWheel = isDrive; } //駆動輪かどうか
	void SetIsFrontWheel(bool isFront) { m_isFrontWheel = isFront; } //前輪かどうか

	//物理計算
	Vector3 CalculateSuspensionForce(const Vector3& vehiclePosition, const Vector4& vehicleRotation); //サスペンションが発生している力(N)
	Vector3 CalculateTireForce(const Vector3& vehicleVelocity, const Vector3& vehicleAngularVelocity); //タイヤが発生している力(N)
	Vector3 CalculateBrakeForce();

	//取得
	float GetRadius() const { return m_radius; }
	float GetRotationSpeed() const { return m_rotationSpeed; } //回転速度(rad/s)
	float GetSuspensionCompression() const { return m_suspensionCompression; } //サスペンションの圧縮量(m)
	Vector3 GetLocalPosition() const { return m_localPosition; }
	Vector3 GetWorldPosition() const { return m_worldPosition; }
	Vector3 GetContactNormal() const { return m_contactNormal; }
	bool IsGrounded() const { return m_isGrounded; }
	bool IsSmoking() const { return m_isSmoking; } //スリップしているかどうか
	float GetSlipRatio() const { return m_slipRatio; } //スリップ率

	//描画用
	float GetVisualRotation() const { return m_visualRotation; } //見た目の回転(rad)
	float GetSteerAngle() const { return m_steerAngle; } //操舵角(rad)

private:
	void UpdateGroundContact(const Vector3& vehiclePosition, const Vector4& vehicleRotation);
	void UpdateTirePhysics(const Vector3& vehicleVelocity);
	Vector3 GetForwardDirection() const;
	Vector3 GetSideDirection() const;

	//ホイール基本設定
	float m_radius;			//ホイール半径(m)
	float m_mass;			//ホイール質量(kg)
	Vector3 m_localPosition;	//ホイールの車体に対するローカル位置(m)
	Vector3 m_worldPosition;	//ホイールのワールド位置(m)

	//サスペンション
	float m_suspensionStiffness;	//サスペンション剛性(N/m)
	float m_suspensionDamping;	//サスペンション減衰(Ns/m)
	float m_suspensionRestLength;	//サスペンションの自然長(m)
	float m_suspensionCompression;	//サスペンションの圧縮量(m)
	float m_lastSuspensionLength;	//前フレームのサスペンション長(m)

	//タイヤ特性
	float m_tireGripFront;	//前後方向のタイヤグリップ係数
	float m_tireGripSide;	//横方向のタイヤグリップ係数
	float m_tireFriction;	//タイヤ摩擦係数

	//物理状態
	float m_rotationSpeed;	//ホイールの回転速度(rad/s)
	float m_visualRotation;	//ホイールの見た目の回転(rad)
	float m_steerAngle;		//ホイールの操舵角(rad)
	Vector3 m_contactNormal;	//接地面の法線ベクトル
	Vector3 m_tireForce;		//タイヤが発生している力(N)
	bool m_isGrounded;		//接地しているかどうか
	bool m_isDriveWheel;	//駆動輪かどうか
	bool m_isFrontWheel;	//前輪かどうか

	//パーティクル効果用
	float m_slipRatio;		//スリップ率
	bool m_isSmoking;		//スリップしているかどうか

};
