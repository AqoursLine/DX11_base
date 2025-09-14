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
	, m_frontAxlePosition(1.2f)	//重心から前輪軸までの距離
	, m_rearAxlePosition(-1.5f)	//重心から後輪軸までの距離
	, m_frontBrakeRatio(0.6f)	//前60%、後40%に制動力配分
	, m_antiRollStiffness(2000.0f)
	, m_cornerStiffnessFront(80000.0f)
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
	m_currentSteerAngle = 0.0f;
	m_angularVelocity = 0.0f;

	//ホイールデータ初期化
	InitializeWheelPositions();

	return true;
}

void Vehicle::Update(double deltaTime) {
	if (!m_isEngineRunning) {
		return;
	}

	float dt = static_cast<float>(deltaTime);

	UpdateEngine(dt);
	UpdateSteering(dt);
	UpdateWheelPhysics(dt);
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

void Vehicle::InitializeWheelPositions() {
	//前輪左
	m_wheels[0].position = Vector3(-m_trackWidth * 0.5f, 0.0f, m_frontAxlePosition);
	//前輪右
	m_wheels[1].position = Vector3(m_trackWidth * 0.5f, 0.0f, m_frontAxlePosition);
	//後輪左
	m_wheels[2].position = Vector3(-m_trackWidth * 0.5f, 0.0f, m_rearAxlePosition);
	//後輪右
	m_wheels[3].position = Vector3(m_trackWidth * 0.5f, 0.0f, m_rearAxlePosition);

	//全タイヤを接地状態に設定
	for (int i = 0; i < 4; i++) {
		m_wheels[i].isGrounded = true;
	}
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

	//前輪のステア角を更新
	m_wheels[0].steerAngle = m_currentSteerAngle; //前輪左
	m_wheels[1].steerAngle = m_currentSteerAngle; //前輪右
}

void Vehicle::UpdateWheelPhysics(float deltaTime) {
	//各輪の荷重計算
	CalculateWheelLoads();

	//各輪の速度計算
	CalculateWheelVelocities();

	//各輪の力計算
	CalculateWheelForces();

	//計算した力を車体に反映
	ApplyWheelForces(deltaTime);
}

void Vehicle::UpdatePhysics(float deltaTime) {
	//各種力を計算
	Vector3 frictionForce = CalculateFrictionForce();
	Vector3 airResistance = CalculateAirResistance();

	//デバッグ
	Vector3 oldAccel = m_acceleration;
	std::cout << "=== Physics Debug ===" << std::endl;
	std::cout << "Old Acceleration: (" << oldAccel.x << ", " << oldAccel.y << ", " << oldAccel.z << ") m/s²" << std::endl;
	std::cout << "Friction Force: (" << frictionForce.x << ", " << frictionForce.y << ", " << frictionForce.z << ") N" << std::endl;
	std::cout << "Air Resistance: (" << airResistance.x << ", " << airResistance.y << ", " << airResistance.z << ") N" << std::endl;

	//抵抗力を加算
	m_acceleration += (frictionForce + airResistance) / m_mass;

	//デバッグ
	std::cout << "New Acceleration: (" << m_acceleration.x << ", " << m_acceleration.y << ", " << m_acceleration.z << ") m/s²" << std::endl;

	//速度を更新
	Vector3 oldVelocity = m_velocity;
	m_velocity += m_acceleration * deltaTime;

	//デバッグ
	std::cout << "Old Velocity: (" << oldVelocity.x << ", " << oldVelocity.y << ", " << oldVelocity.z << ") m/s" << std::endl;
	std::cout << "New Velocity: (" << m_velocity.x << ", " << m_velocity.y << ", " << m_velocity.z << ") m/s" << std::endl;

	//速度の大きさを計算
	m_currentSpeed = m_velocity.Length();

	//速度制限
	if (m_currentSpeed > m_maxSpeed) {
		m_velocity.Normalize();
		m_velocity *= m_maxSpeed;
		m_currentSpeed = m_maxSpeed;

		std::cout << "Speed limited to max: " << m_maxSpeed << " m/s" << std::endl;
	}

	std::cout << "Car Forward: (" << GetForwardQ().x << ", " << GetForwardQ().y << ", " << GetForwardQ().z << ")" << std::endl;
	std::cout << "Car Rotation Yaw: " << m_rotation.y << " rad" << std::endl;
	std::cout << "Throttle Input: " << m_throttleInput << "Steeer Input: " << m_steeringInput << " Brake Input: " << m_brakeInput << std::endl;
	std::cout << "=====================" << std::endl;
}

void Vehicle::UpdateMovement(float deltaTime) {
	//位置を更新
	Vector3 currentPosition = m_position;
	Vector3 newPosition = currentPosition + m_velocity * deltaTime;
	m_position = newPosition;

	//回転を更新
	float newYaw = m_angularVelocity * deltaTime;

	//y軸周りの回転クォータニオンを作成
	Vector4 deltaRotation = Vector4::FromAxisAngle(Vector3::UP, newYaw);

	//クォータニオン乗算で回転を更新
	m_quaternion = deltaRotation * m_quaternion;

	//クォータニオンを正規化
	m_quaternion.Normalize();
}

void Vehicle::CalculateWheelLoads() {
	//静的荷重配分
	float frontLoad = m_mass * 9.81f * (std::abs(m_rearAxlePosition) / m_wheelBase);
	float rearLoad = m_mass * 9.81f * (m_frontAxlePosition / m_wheelBase);

	//加速による荷重変化
	float longitudinalAccel = m_acceleration.Dot(GetForwardQ());
	float loadTransfer = (longitudinalAccel * m_mass * m_cgHeight) / m_wheelBase;

	frontLoad -= loadTransfer; //加速時は前輪の荷重減少
	rearLoad += loadTransfer;  //加速時は後輪の荷重増加

	//横加速による荷重移動(簡易版)
	float lateralAccel = m_acceleration.Dot(GetRightQ());
	float lateralLoadTransfer = (lateralAccel * m_mass * m_cgHeight) / m_trackWidth;

	//各輪の荷重を設定
	m_wheels[0].load = (frontLoad * 0.5f) - (lateralLoadTransfer * 0.5f); //前輪左
	m_wheels[1].load = (frontLoad * 0.5f) + (lateralLoadTransfer * 0.5f); //前輪右
	m_wheels[2].load = (rearLoad * 0.5f) - (lateralLoadTransfer * 0.5f);  //後輪左
	m_wheels[3].load = (rearLoad * 0.5f) + (lateralLoadTransfer * 0.5f);  //後輪右

	//荷重の最小値を設定
	for (int i = 0; i < 4; i++) {
		m_wheels[i].load = std::max(m_wheels[i].load, 50.0f); //最低50Nの荷重を確保
	}
}

void Vehicle::CalculateWheelVelocities() {
	Vector3 forward = GetForwardQ();
	Vector3 right = GetRightQ();

	for (int i = 0; i < 4; i++) {
		//車体の並進速度
		Vector3 translationalVelocity = m_velocity;

		//車体の回転による速度成分
//		Vector3 rotationalVelocity = Vector3(0.0f, m_angularVelocity, 0.0f).Cross(m_wheels[i].position);
		Vector3 angularVelocityVec(0.0f, m_angularVelocity, 0.0f);
		Vector3 rotationalVelocity = angularVelocityVec.Cross(m_wheels[i].position);

		//合成速度
		m_wheels[i].velocity = translationalVelocity + rotationalVelocity;
	}
}

void Vehicle::CalculateWheelForces() {
	for (int i = 0; i < 4; i++) {
		if (!m_wheels[i].isGrounded) {
			m_wheels[i].force = Vector3::ZERO;
			continue;
		}

		m_wheels[i].force = CalculateWheelForce(i);
	}
}

void Vehicle::ApplyWheelForces(float deltaTime) {
	Vector3 totalForce = Vector3::ZERO;
	float totalTorque = 0.0f;

	//デバッグ
	std::cout << "=== Wheel Forces Debug ===" << std::endl;

	for (int i = 0; i < 4; i++) {
		std::cout << "Wheel " << i << " Force: (" << m_wheels[i].force.x << ", " << m_wheels[i].force.y << ", " << m_wheels[i].force.z << ") N" << std::endl;
		std::cout << "Wheel " << i << " Velocity: (" << m_wheels[i].velocity.x << ", " << m_wheels[i].velocity.y << ", " << m_wheels[i].velocity.z << ") m/s" << std::endl;
		std::cout << "Wheel " << i << " Steer Angle: " << m_wheels[i].steerAngle << " rad" << std::endl;

		//並進力を合計
		totalForce += m_wheels[i].force;

		//トルクを計算(重心周り)
		Vector3 leverArm = m_wheels[i].position;
		Vector3 torqueVec = leverArm.Cross(m_wheels[i].force);
		totalTorque += torqueVec.y; //y軸周りのトルクのみ考慮
		std::cout << "Wheel " << i << " Torque Contribution: " << torqueVec.y << " Nm" << std::endl;
	}

	//デバッグ
	std::cout << "Total Force: (" << totalForce.x << ", " << totalForce.y << ", " << totalForce.z << ") N" << std::endl;
	std::cout << "Total Torque: " << totalTorque << " Nm" << std::endl;

	//加速度を計算
	m_acceleration = totalForce / m_mass;

	//角加速度を計算(簡易モーメント使用)
	float momentOfInertia = m_mass * (m_wheelBase * m_wheelBase + m_trackWidth * m_trackWidth) / 24.0f;
	float angularAcceleration = totalTorque / momentOfInertia;
	float oldAngularVelocity = m_angularVelocity;
	m_angularVelocity += angularAcceleration * deltaTime;

	//デバッグ
	std::cout << "Angular Acceleration: " << angularAcceleration << " rad/s²" << std::endl;
	std::cout << "Old Angular Velocity: " << oldAngularVelocity << " rad/s" << std::endl;
	std::cout << "New Angular Velocity: " << m_angularVelocity << " rad/s" << std::endl;
	std::cout << "=========================" << std::endl;

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
	Vector3 forwardDirection = GetForwardQ();
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

	Vector3 forward = GetForwardQ();
	Vector3 right = GetRightQ();

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

Vector3 Vehicle::CalculateWheelForce(int wheelIndex) {
	WheelData& wheel = m_wheels[wheelIndex];
	Vector3 wheelForce = Vector3::ZERO;

	//車体座標系でのタイヤ向きに変換
	Vector3 carForward = GetForwardQ();
	Vector3 carRight = GetRightQ();

	float steerAngle = wheel.steerAngle;
	Vector3 worldWheelForward = carForward * std::cos(steerAngle) + carRight * std::sin(steerAngle);
	Vector3 worldWheelRight = carForward * (-std::sin(steerAngle)) + carRight * std::cos(steerAngle);

	//デバッグ
	if (wheelIndex == 0) {
		std::cout << "Wheel 0 Debug:" << std::endl;
		std::cout << "  Steer Angle: " << steerAngle << " rad" << std::endl;
		std::cout << "  World Wheel Forward: (" << worldWheelForward.x << ", " << worldWheelForward.y << ", " << worldWheelForward.z << ")" << std::endl;
	}

	//タイヤ速度を分離
	float longitudinalVel = wheel.velocity.Dot(worldWheelForward);
	float lateralVel = wheel.velocity.Dot(worldWheelRight);

	//スリップ各とスリップ比を計算
	wheel.slipAngle = CalculateSlipAngle(wheelIndex);
	wheel.slipRatio = CalculateSlipRatio(wheelIndex, longitudinalVel);

	//縦力(駆動・制動力)の計算
	float longitudinalForce = 0.0f;
	if (wheelIndex >= 2) {
		//後輪に駆動力を適用
		if (std::abs(m_throttleInput) > 0.01f) {
			float engineForce = m_accelerrationForce * std::abs(m_throttleInput) * 0.5f;
			longitudinalForce = (m_throttleInput > 0.0f) ? engineForce : -engineForce * 0.7f; //後退は70%の力

			//デバッグ
			if (wheelIndex == 2) {
				std::cout << "  Engine Force Applied: " << longitudinalForce << " N" << std::endl;
			}
		}
	}

	//ブレーキ力
	if (m_brakeInput > 0.01f) {
		float brakeForce = m_brakeForce * m_brakeInput;

		//前輪分配
		if (wheelIndex < 2) {
			//前輪
			brakeForce *= m_frontBrakeRatio * 0.5f;
		} else {
			//後輪
			brakeForce *= (1.0f - m_frontBrakeRatio) * 0.5f;
		}

		if (longitudinalVel > 0.1f) {
			longitudinalForce -= brakeForce;
		} else if (longitudinalForce < - 0.1f) {
			longitudinalForce += brakeForce;
		}
	}

	//横力(コーナリングフォース)の計算
	float lateralForce = -wheel.slipAngle * m_cornerStiffnessFront * 0.01f;

	//グリップファクターを計算
	float combinedSlip = std::sqrt(wheel.slipRatio * wheel.slipRatio + wheel.slipAngle * wheel.slipAngle);
	float gripFactor = CalculateGripFactor(combinedSlip, wheel.load);

	//グリップファクターを適用
	longitudinalForce *= gripFactor;
	lateralForce *= gripFactor;

	//グリップ制限
	float maxForce = wheel.load * m_friction;
	float totalForce = std::sqrt(longitudinalForce * longitudinalForce + lateralForce * lateralForce);

	if (totalForce > maxForce) {
		float scale = maxForce / totalForce;
		longitudinalForce *= scale;
		lateralForce *= scale;
	}

	//世界座標家の力に変換
	wheelForce = worldWheelForward * longitudinalForce + worldWheelRight * lateralForce;

	return wheelForce;
}

float Vehicle::CalculateSlipRatio(int wheelIndex, float wheelSpeed) {
	//簡易スリップ比計算
	if (std::abs(m_currentSpeed) < 0.1f) {
		return 0.0f;
	}

	return 0.0f;
}

float Vehicle::CalculateSlipAngle(int wheelIndex) {
	const WheelData& wheel = m_wheels[wheelIndex];

	if (wheel.velocity.Length() < 0.1f) {
		return 0.0f;
	}

	//タイヤの向き
	Vector3 carForward = GetForwardQ();
	Vector3 carRight = GetRightQ();

	float steerAngle = wheel.steerAngle;
	Vector3 wheelForward = carForward * std::cos(steerAngle) + carRight * std::sin(steerAngle);
	Vector3 wheelRight = carForward * (-std::sin(steerAngle)) + carRight * std::cos(steerAngle);

	//スリップ角の計算
	float longitudinalVel = wheel.velocity.Dot(wheelForward);
	float lateralVel = wheel.velocity.Dot(wheelRight);

	if (std::abs(longitudinalVel) < 0.1f) {
		return 0.0f;
	}

	return std::atan2(lateralVel, std::abs(longitudinalVel));
}

float Vehicle::CalculateGripFactor(float slip, float load) {
	//Pacejkaタイヤモデルの簡易版

	//最小グリップ
	float minGrip = 0.3f;

	//正規化された荷重
	float normalizedLoad = load / 2500.0f;
	normalizedLoad = std::clamp(normalizedLoad, 0.1f, 2.0f);

	//荷重依存のピークグリップ
	float peakGrip = 0.8f + 0.4f * (1.0f - std::abs(normalizedLoad - 1.0f));

	//スリップに対するグリップ減少
	float peakSlip = 0.15f;

	if (slip <= peakSlip) {
		//線形領域
		float linearGrip = (slip / peakSlip) * peakGrip;
		return std::max(linearGrip, minGrip);
	} else {
		//非線形領域
		float excessSlip = slip - peakSlip;
		float decay = std::exp(-excessSlip * 8.0f);
		return std::max(peakGrip * decay, minGrip);
	}
}
