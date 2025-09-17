#include "wheel.h"
#include <algorithm>

Wheel::Wheel()
	: m_radius(0.3f)			//ホイール半径(m)
	, m_mass(20.0f)			//ホイール質量(kg)
	, m_suspensionStiffness(25000.0f) //サスペンション剛性(N/m)
	, m_suspensionDamping(3000.0f)	//サスペンション減衰(Ns/m)
	, m_suspensionRestLength(0.4f)	//サスペンションの自然長(m)
	, m_suspensionCompression(0.0f)
	, m_lastSuspensionLength(m_suspensionRestLength)
	, m_tireGripFront(1.2f)	//前後方向のタイヤグリップ係数
	, m_tireGripSide(1.0f)	//横方向のタイヤグリップ係数
	, m_tireFriction(0.8f)	//タイヤ摩擦係数
	, m_rotationSpeed(0.0f)	//ホイールの回転速度(rad/s)
	, m_visualRotation(0.0f)	//ホイールの見た目の回転(rad)
	, m_steerAngle(0.0f)		//ホイールの操舵角(rad)
	, m_isGrounded(false)
	, m_isDriveWheel(false)	//駆動輪かどうか
	, m_isFrontWheel(false)	//前輪かどうか
	, m_slipRatio(0.0f)		//スリップ率
	, m_isSmoking(false)		//スリップしているかどうか
{
	m_contactNormal = Vector3::UP;
	m_tireForce = Vector3::ZERO;
}

void Wheel::Update(double deltaTime, const Vector3& vehicleVelocity, float engineTorque, bool isBraking) {
	UpdateTirePhysics(vehicleVelocity);

	//駆動輪の場合はエンジントルクを適用
	if (m_isDriveWheel && m_isGrounded) {
		float appliedTorque = engineTorque * m_tireFriction; //摩擦係数を考慮
		m_rotationSpeed += (appliedTorque / (m_mass * m_radius * m_radius)) * static_cast<float>(deltaTime); //角加速度 = トルク / 慣性モーメント
	}

	//ブレーキ適用
	if (isBraking && m_isGrounded) {
		float brakeTorque = 2000.0f; //ブレーキトルクの大きさ
		float brakeDeceleration = brakeTorque / (m_mass * m_radius * m_radius); //角減速度 = トルク / 慣性モーメント
		if (m_rotationSpeed > 0) {
			m_rotationSpeed = std::max(0.0f, m_rotationSpeed - brakeDeceleration * static_cast<float>(deltaTime));
		} else {
			m_rotationSpeed = std::min(0.0f, m_rotationSpeed + brakeDeceleration * static_cast<float>(deltaTime));
		}
	}

	//抵抗による減速
	float resistance = 0.1f; //抵抗係数
	m_rotationSpeed *= (1.0f - resistance * static_cast<float>(deltaTime));

	//描画用回転角度更新
	m_visualRotation += m_rotationSpeed * static_cast<float>(deltaTime);
}

void Wheel::SetSuspensionSettings(float stiffness, float damping, float restLength) {
	m_suspensionStiffness = stiffness;
	m_suspensionDamping = damping;
	m_suspensionRestLength = restLength;
}

void Wheel::SetTireSettings(float gripFront, float gripSide, float friction) {
	m_tireGripFront = gripFront;
	m_tireGripSide = gripSide;
	m_tireFriction = friction;
}

Vector3 Wheel::CalculateSuspensionForce(const Vector3& vehiclePosition, const Vector4& vehicleRotation) {
	UpdateGroundContact(vehiclePosition, vehicleRotation);

	if (!m_isGrounded) {
		return Vector3::ZERO;
	}

	//サスペンション力計算
	float springForce = m_suspensionStiffness * m_suspensionCompression; //バネ力
	float dampingForce = m_suspensionDamping * (m_lastSuspensionLength - (m_suspensionRestLength - m_suspensionCompression));

	m_lastSuspensionLength = m_suspensionRestLength - m_suspensionCompression;

	return m_contactNormal * (springForce + dampingForce);
}

Vector3 Wheel::CalculateTireForce(const Vector3& vehicleVelocity, const Vector3& vehicleAngularVelocity) {
	if (!m_isGrounded) {
		return Vector3::ZERO;
	}

	//タイヤ速度計算
	Vector3 wheelVelocity = vehicleVelocity;
	float wheelSpeed = m_rotationSpeed * m_radius;

	//前後方向の力
	float forwardSlip = wheelSpeed - wheelVelocity.Dot(GetForwardDirection());
	float frontForce = forwardSlip * m_tireGripFront * m_tireFriction;

	//横方向の力
	float sideSlip = wheelVelocity.Dot(GetSideDirection());
	float sideForce = -sideSlip * m_tireGripSide * m_tireFriction;

	//最大グリップ制限
	float maxGrip = m_tireFriction * 9.8f * 500.0f; //仮の接地荷重(N)
	float totalForce = std::sqrt(frontForce * frontForce + sideForce * sideForce);
	if (totalForce > maxGrip) {
		float scale = maxGrip / totalForce;
		frontForce *= scale;
		sideForce *= scale;
		m_isSmoking = true; //スリップしている
	} else {
		m_isSmoking = false;
	}

	m_slipRatio = totalForce / maxGrip;

	return GetForwardDirection() * frontForce + GetSideDirection() * sideForce;
}

Vector3 Wheel::CalculateBrakeForce() {
	//ブレーキ力はCalculateTireForceで処理されるため、ここでは追加の制動力を返す
	return Vector3::ZERO;
}

void Wheel::UpdateGroundContact(const Vector3& vehiclePosition, const Vector4& vehicleRotation) {
	//簡単な地面接触判定
	//ここでは平面地面(y=0)と仮定

	//ワールド座標でのホイール位置計算
	Vector4 quat = vehicleRotation;
	m_worldPosition = vehiclePosition + quat.RotateVector(m_localPosition);

	float groundHeight = 0.0f; //地面の高さ
	float wheelBottom = m_worldPosition.y - m_radius; //ホイールの底面の高さ

	if (wheelBottom <= groundHeight) {
		m_isGrounded = true;
		m_suspensionCompression = groundHeight - wheelBottom;
		m_suspensionCompression = std::clamp(m_suspensionCompression, 0.0f, m_suspensionRestLength);
		m_contactNormal = Vector3::UP; //地面の法線は上向きと仮定
	} else {
		m_isGrounded = false;
		m_suspensionCompression = 0.0f;
	}
}

void Wheel::UpdateTirePhysics(const Vector3& vehicleVelocity) {
	//タイヤの物理状態更新（必要に応じて拡張可能）
	
	if (!m_isGrounded) {
		return;
	}

	//車輪速度と車体速度の差からスリップ率を計算
	float wheelLinearSpeed = m_rotationSpeed * m_radius;
	Vector3 wheelForward = GetForwardDirection();
	float vehicleForwardSpeed = vehicleVelocity.Dot(wheelForward);

	//縦スリップ
	float longitudinalSlip = 0.0f;
	if (std::abs(vehicleForwardSpeed) > 0.1f) {
		longitudinalSlip = (wheelLinearSpeed - vehicleForwardSpeed) / std::abs(vehicleForwardSpeed);
	}

	//横スリップ
	Vector3 wheelSide = GetSideDirection();
	float lateralSlip = vehicleVelocity.Dot(wheelSide);

	//スリップ角計算
	float slipAngle = 0.0f;
	if (std::abs(vehicleForwardSpeed) > 0.1f) {
		slipAngle = std::atan2(lateralSlip, std::abs(vehicleForwardSpeed));
	}

	//総合的なスリップ値を計算
	m_slipRatio = std::sqrt(longitudinalSlip * longitudinalSlip + slipAngle * slipAngle);

	//スモーク判定
	m_isSmoking = m_slipRatio > 0.3f;
}

Vector3 Wheel::GetForwardDirection() const {
	//ステアリング角度を考慮した前方向ベクトル
	float cosAngle = std::cos(m_steerAngle);
	float sinAngle = std::sin(m_steerAngle);
	return Vector3(cosAngle, 0.0f, sinAngle);
}

Vector3 Wheel::GetSideDirection() const {
	//ステアリング角度を考慮した横方向ベクトル
	float cosAngle = std::cos(m_steerAngle);
	float sinAngle = std::sin(m_steerAngle);
	return Vector3(-sinAngle, 0.0f, cosAngle);
}
