#include "vehicle.h"

#include <algorithm>

bool Vehicle::Initialize() {
	//タイヤの初期位置設定
	m_wheels[0].localPosition = { -m_trackWidth * 0.5f, -0.4f, m_wheelBase * 0.5f }; //前左
	m_wheels[1].localPosition = { m_trackWidth * 0.5f, -0.4f, m_wheelBase * 0.5f };  //前右
	m_wheels[2].localPosition = { -m_trackWidth * 0.5f, -0.4f, -m_wheelBase * 0.5f }; //後左
	m_wheels[3].localPosition = { m_trackWidth * 0.5f, -0.4f, -m_wheelBase * 0.5f };  //後右

	//初期状態設定
	for (int i = 0; i < 4; i++) {
		m_wheels[i].suspensionLength = 0.2f;
		m_wheels[i].isGrounded = true;
		m_wheels[i].grip = m_params.maxTireGrip;
		m_wheels[i].slipRatio = 0.0f;
		m_wheels[i].velocity = Vector3::ZERO;
		m_wheelRotationAngle[i] = 0.0f;
	}

	m_centerOfMass = { 0.0f, -0.2f, 0.0f };

	return true;
}

void Vehicle::Finalize() {
}

void Vehicle::Update(double deltaTime) {

	//車両物理更新
	UpdatePhysics(deltaTime);

	//位置と回転を更新
	m_position += m_velocity * static_cast<float>(deltaTime);
	m_rotation.y += m_angularVelocity.y * static_cast<float>(deltaTime);

	//タイヤ更新
	UpdateWheels(deltaTime);

	//ロール物理更新
	UpdateRollPhysics(deltaTime);

	//タイヤ回転更新
	UpdateWheelRotations(deltaTime);

	//ロール各を回転に反映
	m_rotation.z = m_rollAngle;
}

void Vehicle::SetEngineForce(float force) {
	m_currentEngineForce = std::clamp(force, -m_params.maxEngineForce, m_params.maxEngineForce);
}

void Vehicle::SetBrakingForce(float brake) {
	m_currentBrakingForce = std::clamp(brake, 0.0f, m_params.maxBrakingForce);
}

void Vehicle::SetSteeringValue(float steering) {
	m_currentSteeringAngle = std::clamp(steering, -m_params.maxSteeringAngle, m_params.maxSteeringAngle);
}

void Vehicle::SetHandbrake(bool active) {
	m_handbrakeActive = active;
}

float Vehicle::GetCurrentSpeed() const {
	float speed = m_velocity.Length() * 3.6f; //m/s to km/h
	return speed;
}

float Vehicle::GetMaxSpeed() const {
	//理論最高速度を計算(空気抵抗との平衡点)
	float maxSpeed = std::sqrt(m_params.maxEngineForce / (0.5f * m_params.dragCoefficient * 1.225f));
	return maxSpeed * 3.6f; //m/s to km/h
}

bool Vehicle::IsDrifting() const {
	//少なくとも一つのタイヤがスリップしているか
	for (int i = 0; i < 4; i++) {
		if (m_wheels[i].slipRatio > m_params.driftThreshold) {
			return true;
		}
	}
	return false;
}

Vector3 Vehicle::GetWheelPosition(int wheelIndex) const {
	if (wheelIndex < 0 || wheelIndex >= 4) return Vector3::ZERO;
	return m_wheels[wheelIndex].worldPosition;
}

Vector3 Vehicle::GetWheelRotation(int wheelIndex) const {
	if (wheelIndex < 0 || wheelIndex >= 4) return Vector3::ZERO;

	Vector3 wheelRot = m_rotation;

	//フロントタイヤにステアリング角を適用
	if (wheelIndex < 2) {
		wheelRot.y += m_currentSteeringAngle;
	}

	//タイヤの回転を適用
	wheelRot.x = m_wheelRotationAngle[wheelIndex];

	//サスペンション長が自然長から伸びている場合のみロール回転を適用
	if (m_wheels[wheelIndex].suspensionLength < m_params.maxSuspensionTravel * 0.9f) {
		wheelRot.z = m_rollAngle;
	} else {
		wheelRot.z = 0.0f;
	}

	return wheelRot;
}

float Vehicle::GetWheelSlipRatio(int wheelIndex) const {
	if (wheelIndex < 0 || wheelIndex >= 4) return 0.0f;
	return m_wheels[wheelIndex].slipRatio;
}

void Vehicle::UpdatePhysics(double deltaTime) {
	//力の計算と適用
	CalculateForces(deltaTime);
	ApplyForces(deltaTime);

	//タイヤグリップ更新
	UpdateTireGrip();

	//サスペンション計算
	CalculateSuspension();
}

void Vehicle::UpdateWheels(double deltaTime) {
	UpdateWheelPosition();

	for (int i = 0; i < 4; i++) {
		//タイヤのワールド速度を計算
		Vector3 wheelWorldPos = m_wheels[i].worldPosition;
		Vector3 relativePos = wheelWorldPos - (m_position + m_centerOfMass);

		//車体の回転による速度成分を計算
		Vector3 rotationalVelocity = m_angularVelocity.Cross(relativePos);
		m_wheels[i].velocity = m_velocity + rotationalVelocity;

		//タイヤの方向ベクトルを更新
		float steerAngle = (i < 2) ? m_currentSteeringAngle : 0.0f; //前輪のみステアリング角を適用
		float totalYawAngle = m_rotation.y + steerAngle;

		//前方向ベクトル
		m_wheels[i].forwardDir = { std::sin(totalYawAngle), 0.0f, std::cos(totalYawAngle) };

		//右方向ベクトル(ロールも考慮)
		m_wheels[i].rightDir = { std::cos(totalYawAngle), 0.0f, -std::sin(totalYawAngle) };

		//スリップ率を計算
		Vector3 wheelVel = m_wheels[i].velocity;
		m_wheels[i].slipRatio = CalculateSlipRatio(wheelVel, m_wheels[i].forwardDir);
	}
}

void Vehicle::CalculateForces(double deltaTime) {
	Vector3 totalForce = Vector3::ZERO;
	Vector3 totalTorque = Vector3::ZERO;

	//各タイヤの力を計算
	for (int i = 0; i < 4; i++) {
		//接地していないタイヤはスキップ
		if (!m_wheels[i].isGrounded) continue;

		Vector3 tireForce = CalculateTireForce(i, m_wheels[i].velocity);

		//ブレーキ力を適用
		if (m_currentBrakingForce > 0.0f) {
			Vector3 brakeDirection = -m_wheels[i].velocity;
			brakeDirection.Normalize();
			Vector3 brakeForce = brakeDirection * m_currentBrakingForce * 0.25f; //4輪で分割
			tireForce += brakeForce;
		}

		//ハンドブレーキ力を後輪に適用
		if (m_handbrakeActive && i >= 2) {
			Vector3 handbrakeDirection = -m_wheels[i].velocity;
			if (handbrakeDirection.Length() > 0.01f) {
				handbrakeDirection.Normalize();
				Vector3 handbrakeForce = handbrakeDirection * m_params.maxBrakingForce * 0.3f;
				tireForce += handbrakeForce;
			}
		}

		totalForce += tireForce;

		//トルク計算
		Vector3 leverArm = m_wheels[i].localPosition;
		//ローカル座標をワールド座標系でのトルク計算用に回転
		float cosY = std::cos(m_rotation.y);
		float sinY = std::sin(m_rotation.y);
		Vector3 worldLeverArm;
		worldLeverArm.x = leverArm.x * cosY - leverArm.z * sinY;
		worldLeverArm.y = leverArm.y;
		worldLeverArm.z = leverArm.x * sinY + leverArm.z * cosY;

		Vector3 torque = worldLeverArm.Cross(tireForce);
		totalTorque += torque;
	}

	//エンジン力を後輪に適用
	if (std::abs(m_currentEngineForce) > 0.1f) {
		Vector3 forwardDir = { std::sin(m_rotation.y), 0.0f, std::cos(m_rotation.y) };
		Vector3 engineForce = forwardDir * m_currentEngineForce;
		totalForce += engineForce;
	}

	//空気抵抗を追加
	totalForce += CalculateAirResistance();

	//転がり抵抗を追加
	totalForce += CalculateRollingResistance();

	//加速度計算
	m_acceleration = totalForce / m_params.mass;

	//角加速度を計算
	Vector3 angularAcceleration = totalTorque / (m_params.mass * 0.5f);
	m_angularVelocity += angularAcceleration * static_cast<float>(deltaTime);

	//角速度に減衰を適用
	m_angularVelocity *= 0.95f;

}

void Vehicle::ApplyForces(double deltaTime) {
	//速度更新
	m_velocity += m_acceleration * static_cast<float>(deltaTime);

	//最小速度での摩擦を減らす
	if (m_velocity.Length() > 0.01f) {
		m_velocity *= 0.999f;
	}
}

void Vehicle::UpdateTireGrip() {
	for (int i = 0; i < 4; i++) {
		float slipRatio = std::abs(m_wheels[i].slipRatio);
		float slipAngle = CalculateSlipAngle(m_wheels[i].velocity, m_wheels[i].forwardDir, m_wheels[i].rightDir);

		//グリップ係数を計算
		float gripMultiplier = GetGripMultiplier(slipRatio, slipAngle, m_handbrakeActive && i >= 2);
		m_wheels[i].grip = m_params.maxTireGrip * gripMultiplier;
	}
}

void Vehicle::CalculateSuspension() {
	//サスペンションの伸縮を計算
	for (int i = 0; i < 4; i++) {
		//地面との接触判定	(簡易的にy = 0平面とする)
		float groundHeight = 0.0f;
		float wheelHeight = m_wheels[i].worldPosition.y;

		if (wheelHeight <= groundHeight + 0.1f) {
			m_wheels[i].isGrounded = true;
			m_wheels[i].suspensionLength = std::max(0.0f, wheelHeight - groundHeight);
		} else {
			m_wheels[i].isGrounded = false;
			m_wheels[i].suspensionLength = m_params.maxSuspensionTravel;
		}
	}
}

void Vehicle::UpdateRollPhysics(double deltaTime) {
	//横方向の加速度を計算
	float lateralAccel = CalculateLateralAcceleration();

	//遠心力によるロールモーメントを計算
	float rollMoment = CalculateRollMoment();

	//横方向加速度によるロールモーメント
	float centrifugalRollMoment = lateralAccel * m_params.mass * m_params.centerOfMassHeight;
	rollMoment += centrifugalRollMoment;

	//ロール復元力
	float restoreForce = -m_rollAngle * m_params.rollStiffness;

	//ロール減衰力
	float dampingForce = -m_rollVelocity * m_params.rollDamping;

	//総ロールモーメント
	float totalRollMoment = rollMoment + restoreForce + dampingForce;

	//ロール慣性モーメント
	float rollInertia = m_params.mass * m_trackWidth * m_trackWidth * 0.1f;

	//ロール角加速度
	float rollAcceleration = totalRollMoment / rollInertia;

	//ロール角速度と角度を更新
	m_rollVelocity += rollAcceleration * static_cast<float>(deltaTime);
	m_rollAngle += m_rollVelocity * static_cast<float>(deltaTime);

	//ロール角を制限
	m_rollAngle = std::clamp(m_rollAngle, -m_params.maxRollAngle, m_params.maxRollAngle);

	//減衰を適用
	m_rollVelocity *= 0.98f;
}

Vector3 Vehicle::CalculateTireForce(int wheelIndex, const Vector3& wheelVelocity) {
	const WheelInfo& wheel = m_wheels[wheelIndex];

	if (!wheel.isGrounded) return Vector3::ZERO;

	//横宝庫の力(コーナリングフォース)
	Vector3 lateralVelocity = wheel.rightDir * wheel.velocity.Dot(wheel.rightDir);
	Vector3 lateralForce = -lateralVelocity * wheel.grip * 1000.0f;

	//縦方向の力(駆動/制動制限)
	Vector3 longitudinalVelocity = wheel.forwardDir * wheel.velocity.Dot(wheel.forwardDir);
	Vector3 longitudinalForce = -longitudinalVelocity * wheel.grip * 500.0f;

	//合成力を制限
	Vector3 totalForce = lateralForce + longitudinalForce;
	float maxForce = wheel.grip * m_params.mass * 9.81f * 0.25f; //タイヤ1本あたりの最大力

	if (totalForce.Length() > maxForce) {
		totalForce.Normalize();
		totalForce *= maxForce;
	}

	return totalForce;
}

float Vehicle::CalculateSlipRatio(const Vector3& wheelVelocity, const Vector3& wheelForward) {
	float forwardVel = wheelVelocity.Dot(wheelForward);
	float wheelSpeed = std::abs(forwardVel);

	if (wheelSpeed < 0.1f) return 0.0f;

	//簡略化したスリップ率計算
	float slipRatio = std::abs(forwardVel) / (wheelSpeed + 1.0f);
	return std::clamp(slipRatio, 0.0f, 1.0f);
}

float Vehicle::CalculateSlipAngle(const Vector3& wheelVelocity, const Vector3& wheelForward, const Vector3& wheelRight) {
	float forwardVel = wheelVelocity.Dot(wheelForward);
	float lateralVel = wheelVelocity.Dot(wheelRight);

	if (std::abs(forwardVel) < 0.1f) return 0.0f;

	return std::atan2(lateralVel, std::abs(forwardVel));
}

float Vehicle::GetGripMultiplier(float slipRatio, float slipAngle, bool handbrakeActive) {
	//ハンドブレーキが有効な場合はグリップを大幅に低減
	if (handbrakeActive) {
		return m_params.handbrakeGripReduction;
	}

	//スリップ率とスリップ角度に基づくグリップ低減
	float gripRatio = 1.0f;

	if (slipRatio > m_params.slipThreshold) {
		float excessSlip = slipAngle - m_params.slipThreshold;
		float maxExcess = 1.0f - m_params.slipThreshold;
		float reduction = excessSlip / maxExcess;

		gripRatio = 1.0f - reduction * (1.0f - m_params.minTireGrip / m_params.maxTireGrip);
	}

	//スリップ角による追加低減
	float angleEffect = std::abs(slipAngle) / (XM_PI * 0.25f); //約45度で最大低減
	angleEffect = std::clamp(angleEffect, 0.0f, 1.0f);
	gripRatio *= (1.0f - angleEffect * 0.3f);

	return std::clamp(gripRatio, m_params.minTireGrip / m_params.maxTireGrip, 1.0f);
}

void Vehicle::UpdateWheelPosition() {
	//車体の回転行列を作成
	float cosY = std::cos(m_rotation.y);
	float sinY = std::sin(m_rotation.y);

	for (int i = 0; i < 4; i++) {
		//ローカル位置をワールド座標に変換
		Vector3 localPos = m_wheels[i].localPosition;
		Vector3 rotatedPos;

		//Y軸回転
		rotatedPos.x = localPos.x * cosY + localPos.z * sinY;
		rotatedPos.y = localPos.y;
		rotatedPos.z = -localPos.x * sinY + localPos.z * cosY;

		m_wheels[i].worldPosition = m_position + rotatedPos;
	}
}

void Vehicle::UpdateWheelRotations(double deltaTime) {
	for (int i = 0; i < 4; i++) {
		//タイヤの回転速度を計算
		float wheelRadius = 0.5f; //タイヤ半径(仮定)
		float forwardSpeed = m_wheels[i].velocity.Dot(m_wheels[i].forwardDir);
		float angularVel = forwardSpeed / wheelRadius; //rad/s

		m_wheelRotationAngle[i] += angularVel * static_cast<float>(deltaTime);

		//角度を0～2πにラップ
		while (m_wheelRotationAngle[i] > XM_2PI) {
			m_wheelRotationAngle[i] -= XM_2PI;
		}
		while (m_wheelRotationAngle[i] < 0.0f) {
			m_wheelRotationAngle[i] += XM_2PI;
		}
	}
}

Vector3 Vehicle::CalculateAirResistance() const {
	//低速時は空気抵抗をなし
	if (m_velocity.Length() < 0.1f) return Vector3::ZERO;

	Vector3 airResistance = -m_velocity;
	float speed = m_velocity.Length();
	airResistance.Normalize();

	//空気抵抗　= 0.5 * p * Cd * A * v^2
	airResistance *= 0.5f * 1.225f * m_params.dragCoefficient * 2.5f * speed * speed;

	return airResistance;
}

Vector3 Vehicle::CalculateRollingResistance() const {
	//低速時は転がり抵抗をなし
	if (m_velocity.Length() < 0.1f) return Vector3::ZERO;

	Vector3 rollingResistance = -m_velocity;
	rollingResistance.Normalize();

	//転がり抵抗 = Crr * m * g
	rollingResistance *= m_params.rollingResistance * m_params.mass * 9.81f;
	return rollingResistance;
}

float Vehicle::CalculateLateralAcceleration() const {
	//車体の右方向ベクトルを計算
	float cosY = std::cos(m_rotation.y);
	float sinY = std::sin(m_rotation.y);
	Vector3 rightDir = { cosY, 0.0f, -sinY };

	//横方向の速度成分
	float lateralVelocity = m_velocity.Dot(rightDir);

	//角速度による遠心加速度
	float angularVel = m_angularVelocity.y;
	float foewardVel = m_velocity.Dot({ sinY, 0.0f, cosY });
	float centripetal = angularVel * foewardVel;

	//総横方向加速度
	return lateralVelocity * 10.0f + centripetal;
}

float Vehicle::CalculateRollMoment() const {
	float rollMoment = 0.0f;

	//各タイヤのロールモーメントを計算
	for (int i = 0; i < 4; i++) {
		if (!m_wheels[i].isGrounded) continue;

		//タイヤの横方向の力
		Vector3 rightdir = m_wheels[i].rightDir;
		float lateralForce = m_wheels[i].velocity.Dot(rightdir) * m_wheels[i].grip * -1000.0f;

		//レバーアーム(タイヤから重心までの距離)
		float leverArm = m_wheels[i].localPosition.x;

		//ロールモーメントに寄与
		rollMoment += lateralForce * leverArm;
	}

	return rollMoment;
}

