#include "vehicle.h"
#include <algorithm>

Vehicle::Vehicle()
	: m_throttleInput(0.0f)
	, m_steeringInput(0.0f)
	, m_brakeInput(0.0f)
	, m_velocity(0.0f, 0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f, 0.0f)
	, m_currentSpeed(0.0f)
	, m_mass(1000.0f) //平均的な車の質量
	, m_engineRPM(800.0f)
	, m_maxRPM(7000.0f)
	, m_idleRPM(800.0f)
	, m_enginePower(220.0f) // 220kW
	, m_maxSpeed(55.0f) //約200km/h
	, m_accelerrationForce(7000.0f)
	, m_brakeForce(8000.0f)
	, m_friction(0.8f)
	, m_airResistance(0.3f)
	, m_rollingResistance(0.012f)
	, m_maxSteerAngle(0.6f) //約34度
	, m_steerSpeed(0.5f) //rad/s
	, m_currentSteerAngle(0.0f)
	, m_lateralGrip(0.9f)
	, m_underSteerGradient(0.4f)
	, m_wheelBase(2.7f)
	, m_trackWidth(1.5f)
	, m_cgHeight(0.5f)
	, m_isEngineRunning(false)
	, m_gearRatio(3.5f)
	, m_angularVelocity(0.0f)
	, m_lateralVelocity(0.0f, 0.0f, 0.0f)
{
}

bool Vehicle::Initialize() {
	m_isEngineRunning = true; //エンジン始動
	m_engineRPM = m_idleRPM;
	m_velocity = Vector3::ZERO;
	m_currentSpeed = 0.0f;

	return true;
}

void Vehicle::Update(double deltaTime) {
	if (!m_isEngineRunning) {
		return;
	}

	float dt = static_cast<float>(deltaTime);

	UpdateEngine(dt);
	UpdateSteering(dt);
	UpdatePhysics(dt);
	UpdateMovement(dt);

}

void Vehicle::Finalize() {
	m_isEngineRunning = false;
}

void Vehicle::SetThrottle(float throttle) {
	m_throttleInput = std::clamp(throttle, -1.0f, 1.0f);
}

void Vehicle::SetSteering(float streering) {
	m_steeringInput = std::clamp(streering, -1.0f, 1.0f);
}

void Vehicle::SetBrake(float brake) {
	m_brakeInput = std::clamp(brake, 0.0f, 1.0f);
}

void Vehicle::UpdateEngine(float deltaTime) {
	//目標RPMを計算
	float targetRPM = m_idleRPM;

	if (m_throttleInput > 0.0f) {
		//前進時のRPM計算
		float speedRatio = m_currentSpeed / m_maxSpeed;
		float throttleRPM = m_idleRPM + (m_maxRPM - m_idleRPM) * m_throttleInput;
		float speedRPM = m_idleRPM + (m_maxRPM - m_idleRPM) * speedRatio * 0.8f;
		targetRPM = std::max(throttleRPM, speedRPM);
	} else if (m_throttleInput < 0.0f) {
		//後退時のRPM計算
		targetRPM = m_idleRPM + (m_maxRPM - m_idleRPM) * std::abs(m_throttleInput) * 0.5f;
	}

	targetRPM = std::clamp(targetRPM, m_idleRPM, m_maxRPM);

	//RPMを徐々に変化させる
	float rpmChangeRate = 3000.0f; //1秒あたりのRPM変化量
	if (targetRPM > m_engineRPM) {
		m_engineRPM = std::min(m_engineRPM + rpmChangeRate * deltaTime, targetRPM);
	} else {
		m_engineRPM = std::max(m_engineRPM - rpmChangeRate * deltaTime, targetRPM);
	}

}

void Vehicle::UpdateSteering(float deltaTime) {
	//目標ステア角を計算
	float targetSteerAngle = m_steeringInput * m_maxSteerAngle;

	//デバッグ用
	if (std::abs(m_steeringInput) > 0.0f) {
		m_currentSteerAngle = m_currentSteerAngle;
	}

	//ステア角を徐々に変化させる
	float steerDifference = targetSteerAngle - m_currentSteerAngle;
	float maxSteerChange = m_steerSpeed * deltaTime;

	if (std::abs(steerDifference) <= maxSteerChange) {
		m_currentSteerAngle = targetSteerAngle;
	} else {
		float steerDirection = (steerDifference > 0.0f) ? 1.0f : -1.0f;
		m_currentSteerAngle += steerDirection * maxSteerChange;
	}

	//ステア角の制限
	m_currentSteerAngle = std::clamp(m_currentSteerAngle, -m_maxSteerAngle, m_maxSteerAngle);
}

void Vehicle::UpdatePhysics(float deltaTime) {
	//各種力を計算
	Vector3 engineForce = CalculateEngineForce();
	Vector3 brakeForce = CalculateBrakeForce();
	Vector3 frictionForce = CalculateFrictionForce();
	Vector3 airResistance = CalculateAirResistance();
	Vector3 lateralForce = CalculateLateralForce();

	//合力を計算
	Vector3 totalForce = engineForce + brakeForce + frictionForce + airResistance + lateralForce;

	//加速度を計算
	m_acceleration = totalForce / m_mass;

	//速度を更新
	m_velocity += m_acceleration * deltaTime;

	//角速度を計算
	if (m_currentSpeed > 0.1f && std::abs(m_currentSteerAngle) > 0.001f) {
		float turnRadius = CalculateTurnRadius();
		if (turnRadius > 0.0f) {
			float angularVelMagnitude = m_currentSpeed / turnRadius;
			m_angularVelocity = (m_currentSteerAngle > 0.0f) ? angularVelMagnitude : -angularVelMagnitude;

			//アンダーステア効果を適用
			m_angularVelocity *= (1.0f - m_underSteerGradient * (m_currentSpeed / m_maxSpeed));
		}
	} else {
		m_angularVelocity *= 0.95f; //速度が低い場合は角速度を減衰
	}

	//速度の大きさを計算
	m_currentSpeed = m_velocity.Length();

	//速度制限
	if (m_currentSpeed > m_maxSpeed) {
		m_velocity.Normalize();
		m_velocity *= m_maxSpeed;
		m_currentSpeed = m_maxSpeed;
	}
}

void Vehicle::UpdateMovement(float deltaTime) {
	//位置を更新
	Vector3 currentPosition = m_position;
	Vector3 newPosition = currentPosition + m_velocity * deltaTime;
	m_position = newPosition;

	//回転を更新
	Vector3 currentRotation = m_rotation;
	float newYaw = currentRotation.y + m_angularVelocity * deltaTime;
	m_rotation.y = newYaw;

	//速度ベクトルを車両の向きに合わせて調整
	if (m_currentSpeed > 0.1f) {
		Vector3 forward = GetForward();
		Vector3 right = GetRight();

		//前進成分と横滑り成分を分離
		float forwardSpeed = m_velocity.Dot(forward);
		float lateralSpeed = m_velocity.Dot(right);

		//横滑りを制限
		float maxLateralSpeed = m_currentSpeed * 0.3f;
		lateralSpeed = std::clamp(lateralSpeed, -maxLateralSpeed, maxLateralSpeed);

		//速度ベクトルを再構築
		m_velocity = forward * forwardSpeed + right * lateralSpeed;
	}
}

Vector3 Vehicle::CalculateEngineForce() {
	if (std::abs(m_throttleInput) < 0.01f) {
		return Vector3::ZERO;
	}

	//エンジン効率カーブ
	float rpmRatio = (m_engineRPM - m_idleRPM) / (m_maxRPM - m_idleRPM);
	float efficiency = 1.0f - std::abs(rpmRatio - 0.7f) * 0.5f; //ピーク効率を7000rpm付近に設定
	efficiency = std::clamp(efficiency, 0.3f, 1.0f);

	//エンジン出力(N)
	float engineForceAmount = m_accelerrationForce * std::abs(m_throttleInput) * efficiency;

	//前進/後退の方向を決定
	Vector3 forwardDirection = GetForward();
	if (m_throttleInput < 0.0f) {
		engineForceAmount *= 0.7f; //後退は前進の70%の力
		forwardDirection = -forwardDirection; //後退
	}

	return forwardDirection * engineForceAmount;
}

Vector3 Vehicle::CalculateBrakeForce() {
	if (m_brakeInput < 0.01f || m_currentSpeed < 0.1f) {
		return Vector3::ZERO;
	}

	//制動力(N)
	float brakeForceAmount = m_brakeForce * m_brakeInput;

	//速度の逆方向に力をかける
	Vector3 brakeDirection = -m_velocity;
	brakeDirection.Normalize();

	return brakeDirection * brakeForceAmount;
}

Vector3 Vehicle::CalculateFrictionForce() {
	if (m_currentSpeed < 0.01f) {
		return Vector3::ZERO;
	}

	//転がり抵抗力(N)
	Vector3 rollingResistanceForce = -m_velocity;
	rollingResistanceForce.Normalize();
	rollingResistanceForce *= m_rollingResistance * m_mass * 9.81f; //mg * μ

	return rollingResistanceForce;
}

Vector3 Vehicle::CalculateAirResistance() {
	if (m_currentSpeed < 0.1f) {
		return Vector3::ZERO;
	}

	//空気抵抗力(N)
	Vector3 airResistanceForce = -m_velocity;
	airResistanceForce.Normalize();

	float airResistanceAmount = m_airResistance * m_currentSpeed * m_currentSpeed;
	airResistanceForce *= airResistanceAmount;

	return airResistanceForce;
}

Vector3 Vehicle::CalculateSteeringForce() {
	//将来拡張用
	return Vector3::ZERO;
}

Vector3 Vehicle::CalculateLateralForce() const {
	if (m_currentSpeed < 0.1f) {
		return Vector3::ZERO;
	}

	Vector3 forward = GetForward();
	Vector3 right = GetRight();

	//現在の速度方向と車体方向の差を計算
	Vector3 velocityDir = m_velocity;
	velocityDir.Normalize();

	//横滑り各を計算
	float slipAngle = velocityDir.Dot(right);

	//横方向力を計算
	float lateralForceAmount = -slipAngle * m_lateralGrip * m_mass * 9.81f; //mg * グリップ係数

	//速度に応じてグリップを調整
	float speedFactor = 1.0f - (m_currentSpeed / m_maxSpeed) * 0.3f;
	lateralForceAmount *= speedFactor;

	return right * lateralForceAmount;
}

float Vehicle::CalculateSteerAngle() const {
	//将来拡張用
	return m_currentSteerAngle;
}

float Vehicle::CalculateTurnRadius() const {
	if (std::abs(m_currentSteerAngle) < 0.001f) {
		return 0.0f; //直進
	}
	return m_wheelBase / std::tan(std::abs(m_currentSteerAngle));
}
