#include "vehicle.h"
#include <algorithm>

Vehicle::Vehicle() {
	m_params = VehicleParams();
}

bool Vehicle::Initialize() {
	m_scale = m_params.chassisSize;

	return true;
}

void Vehicle::Finalize() {
}

void Vehicle::Update(double deltaTime) {
	//物理演算を更新
	UpdatePhysics(deltaTime);

	//ホイールの回転を更新
	UpdateWheelRotation(deltaTime);
}

void Vehicle::SetEngineForce(float force) {
	m_currentEngineForce = std::clamp(force, -m_params.maxEngineForce, m_params.maxEngineForce);
}

void Vehicle::SetSteeringValue(float steering) {
	m_currentSteering = std::clamp(steering, -m_params.maxSteeringAngle, m_params.maxSteeringAngle);
}

void Vehicle::SetBrakingForce(float brake) {
	m_currentBrakingForce = std::clamp(brake, 0.0f, m_params.maxBrakingForce);
}

void Vehicle::SetHandbrake(bool handbrake) {
	m_handbrakeActive = handbrake;
}

Vector3 Vehicle::GetWheelPosition(int wheelIndex) const {
	Vector3 wheelOffset;
	switch (wheelIndex) {
		case FRONT_LEFT:
			wheelOffset = m_params.frontLeftWheelPos;
			break;
		case FRONT_RIGHT:
			wheelOffset = m_params.frontRightWheelPos;
			break;
		case REAR_LEFT:
			wheelOffset = m_params.rearLeftWheelPos;
			break;
		case REAR_RIGHT:
			wheelOffset = m_params.rearRightWheelPos;
			break;
		default:
			return m_position;
	}

	//ローカル位置をワールド位置に変換
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMVECTOR localPos = XMVectorSet(wheelOffset.x, wheelOffset.y, wheelOffset.z, 1.0f);
	XMVECTOR worldOffset = XMVector3Transform(localPos, rotationMatrix);

	Vector3 worldPos;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&worldPos), worldOffset);

	return m_position + worldPos;
}

Vector3 Vehicle::GetWheelRotation(int wheelIndex) const {
	Vector3 wheelRotation = m_rotation;

	//前輪の場合、ステアリング角度を加算
	if (wheelIndex == FRONT_LEFT || wheelIndex == FRONT_RIGHT) {
		wheelRotation.y += m_currentSteering;
	}

	//ホイールの回転角度を加算
	wheelRotation.x += m_wheelRotationAngle;

	return wheelRotation;
}

void Vehicle::UpdatePhysics(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//ドリフト角度を計算
	CalculateDriftAngle();

	//ビークル状態を更新
	UpdateVehicleState(dt);

	//エンジン力を適用
	ApplyEngineForce(dt);

	//ステアリングを適用
	ApplySteering(dt);

	//横方向の力を計算
	CalculateLateralForce(dt);

	//ハンドブレーキドリフトを適用
	if (m_handbrakeActive) {
		ApplyHandbrakeDrift(dt);
	}

	//摩擦力を適用
	ApplyFriction(dt);

	//空気抵抗を適用
	ApplyAirResistance(dt);

	//位置を更新
	m_position += m_velocity * dt;

	//回転を更新
	m_rotation.y += m_angularVelocity * dt;
	m_rotation.y = WrapAngle(m_rotation.y);

	//現在の速度を更新
	float speedMS = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z); //水平速度
	m_currentSpeed = speedMS * 3.6f; //m/s -> km/h

	//後退中フラグ更新
	Vector3 forward = GetForward();
	float forwardSpeed = forward.x * m_velocity.x + forward.z * m_velocity.z;
	m_isReversing = (forwardSpeed < -0.1f);
}

void Vehicle::ApplyEngineForce(float deltaTime) {
	if (std::abs(m_currentEngineForce) > 0.1f) {
		//前進方向のベクトルを取得
		Vector3 forward = GetForward();

		//エンジン力を速度に変換
		float forceRatio = m_currentEngineForce / m_params.maxEngineForce;
		float accelerationMagnitude = forceRatio * m_params.acceleration;

		//後退の場合は力を減衰
		if (m_currentEngineForce < 0.0f) {
			accelerationMagnitude *= m_params.reverseForceRatio;
		}

		//加速度を適用
		Vector3 engineAcceleration = forward * accelerationMagnitude;
		m_velocity += engineAcceleration * deltaTime;

		//最大速度制限
		float currentSpeedMs = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
		float maxSpeedMs = m_params.maxSpeed / 3.6f; //km/h -> m/s

		if (currentSpeedMs > maxSpeedMs) {
			float ratio = maxSpeedMs / currentSpeedMs;
			m_velocity.x *= ratio;
			m_velocity.z *= ratio;
		}
	}

	//ブレーキ力を適用
	if (m_currentBrakingForce > 0.1f) {
		float brakeForce = m_currentBrakingForce;
		float brakeFactor = 1.0f - (brakeForce / m_params.maxBrakingForce) * m_params.deceleration * deltaTime;
		brakeFactor = std::max(0.0f, brakeFactor);

		m_velocity.x *= brakeFactor;
		m_velocity.z *= brakeFactor;

		//完全停止処理
		float currentSpeedMs = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
		if (currentSpeedMs < 0.5f) {
			m_velocity.x = 0.0f;
			m_velocity.z = 0.0f;
			m_angularVelocity = 0.0f;
		}
	}
}

void Vehicle::ApplySteering(float deltaTime) {
	//前輪のステアリング角を更新
	if (std::fabsf(m_currentSteering) > 0.01f) {
		m_frontWheelSteeringAngle = m_currentSteering;
	} else {
		m_frontWheelSteeringAngle = 0.0f;
	}

	//車体の回転を更新
	if (std::fabsf(m_currentSpeed) > 1.0f) {
		//速度に応じて回転速度を調整
		float speedFactor = std::min(1.0f, std::abs(m_currentSpeed) / 40.0f);

		//ドリフト時はステアリング感度を調整
		float sensitivity = m_params.steeringSensitivity;
		float steeringEffect = m_frontWheelSteeringAngle * sensitivity * speedFactor;

		//後退時は回転を逆にする
		if (m_isReversing) {
			steeringEffect *= -1.0f;
		}

		//ドリフト中はステアリング効果を増加
		if (m_vehicleState == VehicleState::DRIFT_ACTIVE) {
			steeringEffect *= 1.5f;
		}

		//角速度を更新
		m_angularVelocity = steeringEffect;
	} else {
		m_angularVelocity = 0.0f;
	}
}

void Vehicle::ApplyFriction(float deltaTime) {
	//ドリフト時は摩擦を低減
	float frictionCoeff = m_params.friction;

	//ドリフト中は摩擦を大幅に減らす
	if (m_vehicleState == VehicleState::DRIFT_ACTIVE) {
		frictionCoeff *= 0.6f;
	}

	//摩擦による減速を調整(高速域での過度な減速を防ぐ)
	float speedMs = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	float speedKmh = speedMs * 3.6f;

	//maxSpeedの60%,80%を超えると摩擦が減少
	float maxSpeed60 = m_params.maxSpeed * 0.6f;
	float maxSpeed80 = m_params.maxSpeed * 0.8f;

	//高速時は摩擦の影響を減少
	float frictionMultiplier = 1.0f;
	if (speedKmh > maxSpeed80) {
		frictionMultiplier = 0.1f;
	} else if (speedKmh > maxSpeed60) {
		float ratio = (speedKmh - maxSpeed60) / (maxSpeed80 - maxSpeed60);
		frictionMultiplier = 1.0f - ratio * 0.9f;
	}

	float frictionFactor = std::pow(frictionCoeff, deltaTime * 0.5f * frictionMultiplier);
	m_velocity.x *= frictionFactor;
	m_velocity.z *= frictionFactor;

	//角速度の減衰
	float angularFriction =frictionCoeff;
	if (m_vehicleState == VehicleState::DRIFT_ACTIVE) {
		angularFriction += 0.1f; //ドリフト中は回転の減衰を強く
	}
	m_angularVelocity *= std::pow(angularFriction, deltaTime * 0.5f);
}

void Vehicle::ApplyAirResistance(float deltaTime) {
	//空気抵抗を適用
	float speedMs = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	if (speedMs > 0.1f) {
		float resistance = m_params.airResistance * speedMs * deltaTime;
		float resistanceFactor = std::max(0.0f, 1.0f - resistance);

		m_velocity.x *= resistanceFactor;
		m_velocity.z *= resistanceFactor;
	}
}

void Vehicle::UpdateWheelRotation(float deltaTime) {
	//速度からホイールの回転速度を計算
	float speedMs = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);

	//後退時は回転を逆に
	if (m_isReversing) {
		speedMs *= -1.0f;
	}

	//ホイールの円周から回転角度を計算
	float wheelCircumference = 2.0f * XM_PI * m_params.wheelRadius;
	float rotationRate = speedMs / wheelCircumference;

	m_wheelRotationAngle += rotationRate * deltaTime;
	m_wheelRotationAngle = WrapAngle(m_wheelRotationAngle);
}

void Vehicle::UpdateVehicleState(float deltaTime) {
	float dt = static_cast<float>(deltaTime);
	//ビークル状態を更新
	switch (m_vehicleState) {
		case VehicleState::GRIP_DRIVING:
			HandleGripDriving(dt);
			break;
		case VehicleState::DRIFT_INITIATE:
			HandleDriftInitiate(dt);
			break;
		case VehicleState::DRIFT_ACTIVE:
			HandleDriftActive(dt);
			break;
		case VehicleState::DRIFT_RECOVERY:
			HandleDriftRecovery(dt);
			break;
	}
}

void Vehicle::HandleGripDriving(float deltaTime) {
	//ドリフト開始条件
	bool hasSpeed = (m_currentSpeed > m_params.driftThreshold);
	bool hasHandbrake = m_handbrakeActive;
	bool hasSteering = (std::abs(m_currentSteering) > 0.2f);

	if (hasSpeed && hasHandbrake && hasSteering) {
		m_vehicleState = VehicleState::DRIFT_INITIATE;
		m_driftTimer = 0.0f;
		m_rearSlipFactor = 0.3f; //初期スリップ
	}
}

void Vehicle::HandleDriftInitiate(float deltaTime) {
	m_driftTimer += deltaTime;

	//後輪スリップを急激に増加
	m_rearSlipFactor = std::min(1.0f, m_rearSlipFactor + m_params.driftInitiateForce * deltaTime);
	m_driftIntensity = m_rearSlipFactor;

	//0.2秒後にドリフト状態へ移行
	if (m_driftTimer > 0.2f || m_rearSlipFactor > 0.8f) {
		m_vehicleState = VehicleState::DRIFT_ACTIVE;
		m_driftTimer = 0.0f;
	}

	//ドリフト条件が満たされなくなった場合、回復へ移行
	bool hasSpeed = (m_currentSpeed > m_params.driftThreshold * 0.7f);
	bool hasHandbrake = m_handbrakeActive;
	bool hasSteering = (std::abs(m_currentSteering) > 0.1f);

	if (!hasSpeed || !hasHandbrake || !hasSteering) {
		m_vehicleState = VehicleState::DRIFT_RECOVERY;
		m_driftTimer = 0.0f;
	}
}

void Vehicle::HandleDriftActive(float deltaTime) {
	//ドリフト強度を維持
	if (m_handbrakeActive || std::abs(m_currentSteering) > 0.1f) {
		m_rearSlipFactor = std::min(1.0f, m_rearSlipFactor + m_params.driftSustainForce * deltaTime);
	} else {
		m_rearSlipFactor = std::max(0.5f, m_rearSlipFactor - m_params.driftRecoveryRate * deltaTime);
	}

	m_driftIntensity = m_rearSlipFactor;

	//ドリフト回復条件
	bool hasSpeed = (m_currentSpeed > m_params.driftThreshold * 0.5f);
	bool isNearStraight = (std::abs(m_driftAngle) < 0.15f);
	bool noInput = (!m_handbrakeActive && std::abs(m_currentSteering) < 0.1f);

	if (!hasSpeed || (isNearStraight && noInput)) {
		m_vehicleState = VehicleState::DRIFT_RECOVERY;
		m_driftTimer = 0.0f;
	}
}

void Vehicle::HandleDriftRecovery(float deltaTime) {
	//ドリフト強度を段階的に減少
	m_rearSlipFactor = std::max(0.0f, m_rearSlipFactor - m_params.driftRecoveryRate * deltaTime);
	m_driftIntensity = m_rearSlipFactor;

	//ドリフトが完全に回復したらグリップ走行へ移行
	if (m_rearSlipFactor <= 0.05f) {
		m_vehicleState = VehicleState::GRIP_DRIVING;
		m_rearSlipFactor = 0.0f;
		m_driftIntensity = 0.0f;
		m_driftTimer = 0.0f;
	}
}


void Vehicle::CalculateLateralForce(float deltaTime) {
	//横方向の速度を計算
	Vector3 right = GetRight();
	float lateralSpeed = right.x * m_velocity.x + right.z * m_velocity.z;

	//横方向の力を適用してドリフトを制御
	if (m_vehicleState == VehicleState::GRIP_DRIVING) {
		//グリップ走行：前輪の方向に強制的に進む
		Vector3 frontWheelDirection;
		float frontWheelAngle = m_rotation.y + m_frontWheelSteeringAngle;
		frontWheelDirection.x = sinf(frontWheelAngle);
		frontWheelDirection.z = cosf(frontWheelAngle);
		frontWheelDirection.y = 0.0f;

		float currentSpeedMs = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);

		if (currentSpeedMs > 0.1f) {
			float steeringAmount = std::abs(m_frontWheelSteeringAngle);

			if (steeringAmount < 0.1f) {
				//直進時：横滑り補正のみ
				float correction = -lateralSpeed * 0.9f;
				m_velocity.x += right.x * correction * deltaTime;
				m_velocity.z += right.z * correction * deltaTime;
			} else {
				//カーブ時：前輪方向に向かうように補正
				Vector3 frontWheelVelocity = frontWheelDirection * currentSpeedMs;
				float gripStrength = m_params.frontGripStrength;

				m_velocity.x = std::lerp(m_velocity.x, frontWheelVelocity.x, gripStrength * deltaTime);
				m_velocity.z = std::lerp(m_velocity.z, frontWheelVelocity.z, gripStrength * deltaTime);
			}
		}

		//微小な横滑りを完全除去
		float remainingLateralSpeed = right.x * m_velocity.x + right.z * m_velocity.z;
		if (std::abs(remainingLateralSpeed) < 0.02f && std::abs(m_frontWheelSteeringAngle) < 0.05f) {
			Vector3 forward = GetForward();
			float forwardSpeed = forward.x * m_velocity.x + forward.z * m_velocity.z;
			m_velocity.x = forward.x * forwardSpeed;
			m_velocity.z = forward.z * forwardSpeed;
		}

	} else {
		//ドリフト状態：後輪が大きく膨らむ

		//前輪は比較的グリップを保つ
		float frontGripStrength = 0.7f - m_driftIntensity * 0.3f;

		//後輪は大幅にグリップを失う
		float rearGripLoss = m_params.rearGripLoss * m_driftIntensity;

		//横方向の補正を弱くして後部の膨らみを許容
		float correction = -lateralSpeed * (0.4f - rearGripLoss * 0.3f);
		m_velocity.x += right.x * correction * deltaTime;
		m_velocity.z += right.z * correction * deltaTime;

		//ドリフト中は後輪からの横方向の力を加算
		if (std::abs(m_currentSteering) > 0.1f) {
			float rearSlideForce = m_currentSteering = m_driftIntensity * m_params.rearSlipMuktiplier;
			m_velocity.x += right.x * rearSlideForce * deltaTime;
			m_velocity.z += right.z * rearSlideForce * deltaTime;
		}
	}
}

void Vehicle::ApplyHandbrakeDrift(float deltaTime) {
	float brakeFactor = 1.0f - 2.0f * deltaTime;
	brakeFactor = std::max(0.7f, brakeFactor);

	//ドリフト状態でない場合は通常の減速のみ
	if (m_vehicleState == VehicleState::GRIP_DRIVING) {
		m_velocity.x *= brakeFactor;
		m_velocity.z *= brakeFactor;
		return;
	}

	//ステアリングしてるか
	bool isSteering = (std::abs(m_currentSteering) > 0.1f);

	if (isSteering && m_currentSpeed > m_params.driftThreshold) {
		//ステアリング中：ドリフト効果を強化
		Vector3 right = GetRight();

		//後輪スリップによる横方向の力
		float lateralForce = m_currentSteering * m_rearSlipFactor * 1.5f;
		m_velocity.x += right.x * lateralForce * deltaTime;
		m_velocity.z += right.z * lateralForce * deltaTime;

		//車体回転の増加
		float driftTorque = m_currentSteering * m_rearSlipFactor * (m_currentSpeed / 60.0f);
		m_angularVelocity += driftTorque * deltaTime;

		//ドリフト用の適度な減速
		float driftBrakeFactor = 1.0f - 1.0f * deltaTime;
		driftBrakeFactor = std::max(0.8f, driftBrakeFactor);
		m_velocity.x *= driftBrakeFactor;
		m_velocity.z *= driftBrakeFactor;

	} else {
		//直進中:強力な後輪ブレーキとして機能
		m_velocity.x *= brakeFactor;
		m_velocity.z *= brakeFactor;
		m_angularVelocity *= 0.9f; //回転も減衰
	}
}

void Vehicle::CalculateDriftAngle() {
	//ドリフト角度を計算
	Vector3 forward = GetForward();
	Vector3 velocityDir = m_velocity;

	float velosityMagnitude = sqrtf(velocityDir.x * velocityDir.x + velocityDir.z * velocityDir.z);
	if (velosityMagnitude > 0.1f) {
		velocityDir.x /= velosityMagnitude;
		velocityDir.z /= velosityMagnitude;

		//内積から角度を計算
		float dot = forward.x * velocityDir.x + forward.z * velocityDir.z;
		dot = std::clamp(dot, -1.0f, 1.0f);

		//外積から方向を取得
		float cross = forward.x * velocityDir.z - forward.z * velocityDir.x;
		float newDriftAngle = acos(dot);
		if (cross < 0.0f) {
			newDriftAngle = -newDriftAngle;
		}

		m_driftAngle = newDriftAngle;
	} else {
		m_driftAngle = 0.0f;
	}

	//スリップ比を計算
	m_slipRatio = std::abs(m_driftAngle) / (XM_PI / 4.0f);
	m_slipRatio = std::clamp(m_slipRatio, 0.0f, 1.0f);
}
