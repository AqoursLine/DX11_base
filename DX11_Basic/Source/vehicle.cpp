#include "vehicle.h"

Vehicle::Vehicle()
	: m_velocity(Vector3::ZERO)
	, m_angularVelocity(Vector3::ZERO)
	, m_acceleration(Vector3::ZERO)
	, m_centerOfMass(0.0f, 0.0f, 0.0f) // 重心位置（ローカル座標系）
	, m_mass(1200.0f) // 車両質量（kg）
	, m_inertiaTensor(400.0f, 800.0f, 1200.0f) // 慣性テンソル（簡易的な値）
	, m_steerInput(0.0f)
	, m_throttleInput(0.0f)
	, m_brakeInput(0.0f)
	, m_handbrakeInput(0.0f)
	, m_wheelBase(2.5f) // ホイールベース（m）
	, m_trackWidth(1.6f) // トレッド幅（m）
	, m_maxSteerAngle(0.4f) // 最大ステアリング角（ラジアン）
	, m_dragCoefficient(0.3f) // 空気抵抗係数
	, m_frontalArea(2.2f) // 正面投影面積（m^2）
	, m_groundHeight(0.0f)

{
}

Vehicle::WheelRenderInfo Vehicle::GetWheelRenderInfo(int wheelIndex) const {
	if (wheelIndex < 0 || wheelIndex >= static_cast<int>(m_wheels.size())) {
		return {};
	}

	const Wheel& wheel = m_wheels[wheelIndex];
	WheelRenderInfo info;

	//ワールド位置
	info.position = wheel.worldPosition;

	//サスペンション圧縮による位置調整
	if (wheel.isGrounded) {
		float compressionOffset = wheel.compressionRatio * m_suspensions[wheelIndex].maxCompression;
		info.position.y -= compressionOffset;
	}

	//回転角
	info.rotation.x = wheel.rotationAngle;
	info.rotation.y = wheel.steerAngle + m_rotation.y;
	info.rotation.z = m_rotation.z;

	//その他の情報
	info.compressionRatio = wheel.compressionRatio;
	info.isGrounded = wheel.isGrounded;
	info.radius = wheel.radius;
	info.width = wheel.width;
	info.isFrontWheel = wheel.isFrontWheel;
	info.isDrivenWheel = wheel.isDriveWheel;

	return info;
}

std::vector<Vehicle::WheelRenderInfo> Vehicle::GetAllWheelRenderInfo() const {
	std::vector<WheelRenderInfo> infos;
	infos.reserve(m_wheels.size());

	for (size_t i = 0; i < m_wheels.size(); i++) {
		infos.push_back(GetWheelRenderInfo(static_cast<int>(i)));
	}
	return infos;
}

bool Vehicle::Initialize() {
	// タイヤとサスペンションの初期化
	m_wheels.resize(4);
	m_suspensions.resize(4);

	//ホイールの位置設定(前左、前右、後左、後右)
	m_wheels[0].localPosition = Vector3(-m_trackWidth * 0.5f, 0.0f, m_wheelBase * 0.5f); // 前左
	m_wheels[1].localPosition = Vector3(m_trackWidth * 0.5f, 0.0f, m_wheelBase * 0.5f);  // 前右
	m_wheels[2].localPosition = Vector3(-m_trackWidth * 0.5f, 0.0f, -m_wheelBase * 0.5f); // 後左
	m_wheels[3].localPosition = Vector3(m_trackWidth * 0.5f, 0.0f, -m_wheelBase * 0.5f);  // 後右

	//前輪設定
	m_wheels[0].isFrontWheel = true;
	m_wheels[1].isFrontWheel = true;

	//駆動輪設定(全輪駆動)
	m_wheels[0].isDriveWheel = true;
	m_wheels[1].isDriveWheel = true;
	//m_wheels[2].isDriveWheel = true;
	//m_wheels[3].isDriveWheel = true;

	//全タイヤの基本パラメータ設定
	for (auto& wheel : m_wheels) {
		wheel.radius = 0.32f;
		wheel.width = 0.22f;
		wheel.frictionCoefficient = 1.0f;
	}

	//前輪と後輪で異なる特性
	m_wheels[0].maxGripForce = 2800.0f;
	m_wheels[1].maxGripForce = 2800.0f;
	m_wheels[2].maxGripForce = 3200.0f;
	m_wheels[3].maxGripForce = 3200.0f;

	//サスペンション設定
	for (auto& suspension : m_suspensions) {
		suspension.springConstant = 15000.0f;	// バネ定数
		suspension.damperConstant = 1500.0f;	// ダンパー定数
		suspension.restLength = 0.35f;			// 自然長
		suspension.maxCompression = 0.25f;		// 最大圧縮量
		suspension.currentLength = suspension.restLength;
		suspension.lastLength = suspension.restLength;
	}

	//重心位置設定(ローカル座標系)
	m_centerOfMass = Vector3(0.0f, -0.3f, 0.0f);

	m_inertiaTensor = { 2000.0f, 2400.0f, 1200.0f };

	//エンジン設定
	m_engine.maxTorque = 1000.0f;		// 最大トルク (Nm)
	m_engine.idleRPM = 800.0f;			// アイドリング回転数 (RPM)
	m_engine.maxRPM = 7000.0f;			// 最大回転数 (RPM)
	m_engine.currentRPM = m_engine.idleRPM; // 現在の回転数 (RPM)
	m_engine.engineBraking = 50.0f;		// エンジンブレーキ係数

	//空気抵抗設定
	m_dragCoefficient = 0.3f;	// 空気抵抗係数
	m_frontalArea = 2.2f;		// 正面投影面積 (m^2)

	UpdateWheelPositions();

	return true;
}

void Vehicle::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//エンジンパラメータ設定
	m_engine.throttleInput = std::abs(m_throttleInput);

	UpdatePhysics(dt);
	UpdateWheelPositions();
}

void Vehicle::UpdatePhysics(float deltaTime) {
	UpdateEngine(deltaTime);
	CalculateSuspensionForces(deltaTime);
	CalculateWheelForces(deltaTime);
	UpdateAllWheelRotations(deltaTime);
	CalculateAerodynamicsForces();
	IntegrateForces(deltaTime);
}

void Vehicle::UpdateEngine(float deltaTime) {
	//車両からエンジンRPMを計算
	float wheelSpeed = 0.0f;
	int drivenWheelCount = 0;

	for (const auto& wheel : m_wheels) {
		if (wheel.isDriveWheel && wheel.isGrounded) {
			wheelSpeed += std::abs(wheel.angularVelocity * wheel.radius);
			drivenWheelCount++;
		}
	}

	//スロットル入力によるRPM調整
	if (m_engine.throttleInput > 0.0f) {
		float targetRPM = m_engine.idleRPM + (m_engine.maxRPM - m_engine.idleRPM) * m_engine.throttleInput;
		if (targetRPM > m_engine.currentRPM) {
			m_engine.currentRPM += (targetRPM - m_engine.currentRPM) * deltaTime * 8.0f;
		}
	} else {
		if (drivenWheelCount > 0) {
			wheelSpeed /= drivenWheelCount;

			//簡単なギア比
			float gearRatio = 1.0f; // 仮のギア比
			float targetRPM = (wheelSpeed / (m_wheels[0].radius * 2.0f * 3.14159f)) * 60.0f * gearRatio;
			targetRPM = std::max(targetRPM, m_engine.idleRPM);

			//RPMの変化をスムーズに
			float rpmDiff = targetRPM - m_engine.currentRPM;
			m_engine.currentRPM += rpmDiff * deltaTime * 3.0f; // レスポンスの速さ
		} else {
			m_engine.currentRPM += (m_engine.idleRPM - m_engine.currentRPM) * deltaTime * 2.0f;
		}

	}

	m_engine.currentRPM = std::clamp(m_engine.currentRPM, m_engine.idleRPM, m_engine.maxRPM);

}

void Vehicle::CalculateSuspensionForces(float deltaTime) {
	for (int i = 0; i < 4; i++) {
		Wheel& wheel = m_wheels[i];
		Suspension& suspension = m_suspensions[i];

		//地面との距離を計算
		float groundHeight = GetGroundHeight(wheel.worldPosition);
		float wheelBottomHeight = wheel.worldPosition.y - wheel.radius;

		//サスペンションの圧縮量を計算
		float compressionDistance = suspension.restLength - (wheelBottomHeight - groundHeight);
		compressionDistance = std::clamp(compressionDistance, 0.0f, suspension.maxCompression);

		suspension.lastLength = suspension.currentLength;
		suspension.currentLength = suspension.restLength - compressionDistance;

		wheel.compressionRatio = compressionDistance / suspension.maxCompression;
		wheel.isGrounded = (compressionDistance > 0.001f);

		if (wheel.isGrounded) {
			//スプリング力
			float springForce = compressionDistance * suspension.springConstant;

			//ダンパー力
			float compressionVelocity = (suspension.lastLength - suspension.currentLength) / deltaTime;
			float dmaperForce = compressionVelocity * suspension.damperConstant;

			//上向きの力
			float totalForce = springForce + dmaperForce;

			//過度な力を防ぐ
			totalForce = std::clamp(totalForce, 0.0f, m_mass * 9.81f * 2.0f);

			wheel.suspensionForce = Vector3(0.0f, totalForce, 0.0f);
		} else {
			wheel.suspensionForce = Vector3::ZERO;
		}
	}

}

void Vehicle::CalculateWheelForces(float deltaTime) {
	Vector3 totalForce = Vector3::ZERO;
	Vector3 totalTorque = Vector3::ZERO;

	//重力を加える
	Vector3 gravity = Vector3(0.0f, -9.81f * m_mass, 0.0f);
	totalForce += gravity;

	//アッカーマンジオメトリによるステアリング角度調整
	CalculateAckermannSteering();

	//スピン検出と安定化
	float yawRate = std::abs(m_angularVelocity.y);
	float pitchRate = std::abs(m_angularVelocity.x);
	float spinThreshold = 1.5f; // スピン検出閾値
	float pitchThreshold = 1.0f; // ピッチング検出閾値

	bool isSpinning = yawRate > spinThreshold;
	bool isPitching = pitchRate > pitchThreshold;

	//スピン中は安定化力を適用
	float stabilityFactor = 1.0f;
	if (isSpinning || isPitching) {
		float maxRate = std::max(yawRate - spinThreshold, pitchRate - pitchThreshold);
		stabilityFactor = std::max(0.1f, 1.0f - maxRate * 0.3f);
	}

	for (int i = 0; i < 4; i++) {
		Wheel& wheel = m_wheels[i];

		if (!wheel.isGrounded) {
			wheel.tireForce = Vector3::ZERO;
			wheel.angularVelocity *= 0.95f; // 空転時の減衰
			continue;
		}

		//ワールド座標での車輪の速度を計算
		Vector3 wheelWorldPos = wheel.worldPosition;
		Vector3 relativePos = wheelWorldPos - (GetPosition() + m_centerOfMass);
		Vector3 wheelVelocity = m_velocity + m_angularVelocity.Cross(relativePos);

		//タイヤ力を計算
		Vector3 tireForce = CalculateTireForce(wheel, wheelVelocity);

		//慣性時のステアリング追従力を追加
		ApplyInertiaSteeringForce(wheel, wheelVelocity, tireForce);

		//スピン中は後輪の横力を強化
		if (isSpinning && !wheel.isFrontWheel) {
			Vector3 right = GetRight();
			if (wheel.isFrontWheel) {
				float cosSteer = cosf(wheel.steerAngle);
				float sinSteer = sinf(wheel.steerAngle);
				Vector3 forward = GetForward();
				right = forward * -sinSteer + right * cosSteer;
			}

			float lateralVel = wheelVelocity.Dot(right);
			Vector3 antiSpinForce = right * (-lateralVel * 500.0f);
			tireForce += antiSpinForce;
		}

		//駆動力を追加
		if (wheel.isDriveWheel && std::abs(m_throttleInput) > 0.001f) {
			//エンジンRPMから駆動力を計算
			float engineRPMFactor = m_engine.currentRPM / m_engine.maxRPM;
			float throttleFactor = std::abs(m_throttleInput);

			//基準駆動力
			float baseDriveForce = 10000.0f; // 仮の基準駆動力
			float driveForce = baseDriveForce * engineRPMFactor * throttleFactor;

			//スピン中は駆動力を減少
			if (isSpinning) {
				driveForce *= 0.3f;
			}

			//前進後退の方向
			Vector3 driveDirection = GetForward();

			//ステアリング角度を考慮
			if (wheel.isFrontWheel) {
				float cosSteer = cosf(wheel.steerAngle);
				float sinSteer = sinf(wheel.steerAngle);
				driveDirection = driveDirection * cosSteer + GetRight() * sinSteer;
			}

			//後退時は逆方向
			if (m_throttleInput < 0.0f) {
				driveDirection = -driveDirection;
			}

			tireForce += driveDirection * driveForce;
		}

		//ブレーキ力を追加
		if (m_brakeInput > 0.001f) {
			Vector3 brakeDirection = -wheelVelocity;
			brakeDirection.Normalize();
			float brakeForce = m_brakeInput * 8000.0f; // 仮のブレーキ力
			tireForce += brakeDirection * brakeForce;
		}

		//ハンドブレーキ力を追加（後輪のみ）
		if (m_handbrakeInput && !wheel.isFrontWheel) {
			Vector3 handbrakeDirection = -wheelVelocity;
			handbrakeDirection.Normalize();
			float handbrakeForce = 12000.0f; // 仮のハンドブレーキ力
			tireForce += handbrakeDirection * handbrakeForce;
		}

		wheel.tireForce = tireForce;


		//車体に作用する力とトルクを計算
		totalForce += wheel.suspensionForce + tireForce;

		Vector3 wheelRelativePos = wheel.worldPosition - GetPosition();
		Vector3 appliedTorque = wheelRelativePos.Cross(tireForce);

		//ピッチトルクを抑制
		appliedTorque.x *= 0.3f;

		//スピン中はトルクを制限
		appliedTorque *= stabilityFactor;
		totalTorque += appliedTorque;	
	}

	m_acceleration = totalForce / m_mass;

	//車体方向への追従力を追加
	ApplyDirectionTrackingForce();

	//角加速度
	Vector3 angularAcceleration;
	angularAcceleration.x = totalTorque.x / m_inertiaTensor.x;
	angularAcceleration.y = totalTorque.y / m_inertiaTensor.y;
	angularAcceleration.z = totalTorque.z / m_inertiaTensor.z;

	//各加速度制限
	float maxPitchAccel = 0.5f; // 最大ピッチ加速度
	float maxYawAccel = 0.8f;   // 最大ヨー加速度
	float maxRollAccel = 0.6f;  // 最大ロール加速度

	angularAcceleration.x = std::clamp(angularAcceleration.x, -maxPitchAccel, maxPitchAccel);
	angularAcceleration.y = std::clamp(angularAcceleration.y, -maxYawAccel, maxYawAccel);
	angularAcceleration.z = std::clamp(angularAcceleration.z, -maxRollAccel, maxRollAccel);

	m_angularVelocity += angularAcceleration * deltaTime;

}

void Vehicle::UpdateAllWheelRotations(float deltaTime) {
	for (int i = 0; i < static_cast<int>(m_wheels.size()); i++) {
		Wheel& wheel = m_wheels[i];

		//ワールド座標での車輪の速度を計算
		Vector3 wheelWorldPos = wheel.worldPosition;
		Vector3 relativePos = wheelWorldPos - (GetPosition() + m_centerOfMass);
		Vector3 wheelVelocity = m_velocity + m_angularVelocity.Cross(relativePos);

		UpdateWheelRotation(i, wheelVelocity, deltaTime);
	}
}

void Vehicle::CalculateAerodynamicsForces() {
	//空気抵抗
	float speed = m_velocity.Length();
	if (speed > 1.0f) {
		Vector3 dragDirection = -m_velocity;
		dragDirection.Normalize();

		float dragForce = 0.5f * 1.225f * speed * speed * (m_dragCoefficient * 0.1f) * m_frontalArea;
		m_acceleration += dragDirection * (dragForce / m_mass);

		//ダウンフォース
		float downforceCoefficient = 0.8f; // ダウンフォース係数
		float downforce = 0.5f * 1.225f * speed * speed * downforceCoefficient * m_frontalArea;

		//速度の2乗に比例して下向きの力を加える
		Vector3 downforceVec = Vector3(0.0f, -downforce / m_mass, 0.0f);
		m_acceleration += downforceVec;

		//各車輪への荷重分配
		//ダウンフォースを各車輪のグリップに反映
		float wheelDownforce = downforce / 4.0f; // 仮に均等分配
		for (auto& wheel : m_wheels) {
			//速度が上がるほどグリップが向上
			float speedBonus = std::min(speed * 0.05f, 1.5f);
			wheel.maxGripForce = wheel.maxGripForce * (1.0f + speedBonus);
		}
	}
}

void Vehicle::IntegrateForces(float deltaTime) {
	//線形運動の積分
	m_velocity += m_acceleration * deltaTime;
	m_position += m_velocity * deltaTime;

	//回転運動の積分
	Vector3 angularDisplacement = m_angularVelocity * deltaTime;

	//Y軸回転の制限でスピン防止
	float maxYawRate = 2.0f; // 最大ヨーレート (ラジアン/秒)
	if (std::abs(m_angularVelocity.y) > maxYawRate) {
		float sign = m_angularVelocity.y > 0.0f ? 1.0f : -1.0f;
		m_angularVelocity.y = sign * maxYawRate;
		angularDisplacement.y = m_angularVelocity.y * deltaTime;
	}

	//現在の回転に角変位を適用
	m_rotation += angularDisplacement;

	//適度の減衰を加える
	m_velocity *= 0.9995f;

	//角速度も減衰(yaw軸は少し強めに)
	m_angularVelocity.x *= 0.9f;
	m_angularVelocity.y *= 0.92f;
	m_angularVelocity.z *= 0.93f;

	//非常に小さい値は0にして安定化
	if (std::abs(m_angularVelocity.x) < 0.1f) {
		m_angularVelocity.x = 0.0f;
	}
	if (std::abs(m_angularVelocity.y) < 0.1f) {
		m_angularVelocity.y = 0.0f;
	}
	if (std::abs(m_angularVelocity.z) < 0.1f) {
		m_angularVelocity.z = 0.0f;
	}
}

void Vehicle::UpdateWheelPositions() {
	for (int i = 0; i < 4; i++) {
		m_wheels[i].worldPosition = LocalToWorld(m_wheels[i].localPosition);
	}
}

Vector3 Vehicle::CalculateTireForce(const Wheel& wheel, const Vector3& wheelVelocity) {
	Vector3 tireForce = Vector3::ZERO;

	//ローカル座標系でのタイヤ方向を計算
	Vector3 forward = GetForward();
	Vector3 right = GetRight();

	//ステアリング角度を適用(前輪のみ)
	if (wheel.isFrontWheel) {
		float cosSteer = cosf(wheel.steerAngle);
		float sinSteer = sinf(wheel.steerAngle);

		Vector3 steerForward = forward * cosSteer + right * sinSteer;
		Vector3 steerRight = right * cosSteer - forward * sinSteer;

		forward = steerForward;
		right = steerRight;
	}

	//横方向と前方向の速度成分を計算
	float longitudinalVel = wheelVelocity.Dot(forward);
	float lateralVel = wheelVelocity.Dot(right);

	//速度依存のグリップ係数
	float speed = wheelVelocity.Length();

	//車体方向追従用
	Vector3 carForward = GetForward();
	carForward.y = 0.0f;
	carForward.Normalize();

	Vector3 actualDirection = m_velocity;
	actualDirection.y = 0.0f;
	if (actualDirection.Length() > 0.1f) {
		actualDirection.Normalize();
	} else {
		actualDirection = carForward;
	}

	//車体方向とのズレ角度
	float directionDot = carForward.Dot(actualDirection);
	float directionError = 1.0f - directionDot; // 0(一致)～2(逆方向)

	//縦方向の力
	float maxLongitudinalForce = wheel.maxGripForce * wheel.frictionCoefficient;
	//方向のずれが大きいほど強い補正力を適用
	float directionCorrectionFactor = 1.0f + directionError * 3.0f;

	float longitudinalResponse = std::max(100.0f, 200.0f - speed * 3.0f); // 速度に応じたレスポンス
	longitudinalResponse *= directionCorrectionFactor;
	float longitudinalForce = -longitudinalVel * longitudinalResponse;
	longitudinalForce = std::clamp(longitudinalForce, -maxLongitudinalForce, maxLongitudinalForce);

	//横方向の力
	float maxLateralForce = wheel.maxGripForce * wheel.frictionCoefficient;

	float lateralResponse;
	if (wheel.isFrontWheel) {
		//慣性による横滑りを抑制
		float baseResponse = std::max(80.0f, 180.0f - speed * 4.0f);

		//ステアリング入力がある場合は横力を強化
		float steerFactor = 1.0f + std::abs(wheel.steerAngle) * 0.8f;
		lateralResponse = baseResponse * steerFactor;
	} else {
		//車体方向への追従を重視
		lateralResponse = std::max(120.0f, 200.0f - speed * 4.0f);

		//車体方向のズレが大きい場合はさらに横力を強化
		float trackingFactor = 1.0f + directionError * 1.0f;
		lateralResponse *= trackingFactor;
	}

	float lateralForce = -lateralVel * lateralResponse;
	lateralForce = std::clamp(lateralForce, -maxLateralForce, maxLateralForce);

	//速度が0に近い場合の安定化
	if (speed < 0.5f) {
		//停止時は非常に強いフリップで安定させる
		longitudinalForce = -longitudinalVel * 500.0f;
		lateralForce = -lateralVel * 500.0f;

		longitudinalForce = std::clamp(longitudinalForce, -maxLongitudinalForce, maxLongitudinalForce);
		lateralForce = std::clamp(lateralForce, -maxLateralForce, maxLateralForce);
	}

	tireForce = forward * longitudinalForce + right * lateralForce;

	//グリップサークルの制限
	float combinedMagnitude = tireForce.Length();
	float maxCombinedForce = wheel.maxGripForce * wheel.frictionCoefficient;

	//高い閾値で制限
	if (combinedMagnitude > maxCombinedForce) {
		tireForce = tireForce * (maxCombinedForce / combinedMagnitude);
	}

	return tireForce;
}

float Vehicle::CalculateSlipRatio(const Wheel& wheel, const Vector3& wheelVelocity) {
	Vector3 forward = GetForward();

	if (wheel.isFrontWheel) {
		//ステアリング角度を考慮
		float cosSteer = cosf(wheel.steerAngle);
		float sinSteer = sinf(wheel.steerAngle);
		Vector3 right = GetRight();
		forward = forward * cosSteer + right * sinSteer;
	}

	float wheelSpeed = wheel.angularVelocity * wheel.radius;
	float vehicleSpeed = wheelVelocity.Dot(forward);

	if (std::abs(vehicleSpeed) < 0.1f) {
		return 0.0f;
	}

	return (wheelSpeed - vehicleSpeed) / std::abs(vehicleSpeed);
}

float Vehicle::CalculateSlipAngle(const Wheel& wheel, const Vector3& wheelVelocity) {
	Vector3 forward = GetForward();
	Vector3 right = GetRight();

	if (wheel.isFrontWheel) {
		//ステアリング角度を考慮
		float cosSteer = cosf(wheel.steerAngle);
		float sinSteer = sinf(wheel.steerAngle);
		forward = forward * cosSteer + right * sinSteer;
		right = right * cosSteer - forward * sinSteer;
	}

	float longitudinalVel = wheelVelocity.Dot(forward);
	float lateralVel = wheelVelocity.Dot(right);

	if (std::abs(longitudinalVel) < 0.1f) {
		return 0.0f;
	}

	return atan2f(lateralVel, std::abs(longitudinalVel));
}

void Vehicle::UpdateWheelRotation(int wheelIndex, const Vector3& wheelVelocity, float deltaTime) {
	if (wheelIndex < 0 || wheelIndex >= static_cast<int>(m_wheels.size())) {
		return;
	}

	Wheel& wheel = m_wheels[wheelIndex];

	//車輪の前進方向を計算
	Vector3 wheelForward = GetForward();

	//前輪の場合はステアリング角度を考慮
	if (wheel.isFrontWheel) {
		float cosSteer = cosf(wheel.steerAngle);
		float sinSteer = sinf(wheel.steerAngle);
		Vector3 right = GetRight();
		wheelForward = wheelForward * cosSteer + right * sinSteer;
	}

	//車輪の前進方向の速度成分を計算
	float wheelSpeed = wheelVelocity.Dot(wheelForward);

	//角速度を計算
	float targetAngularVelocity = wheelSpeed / wheel.radius;

	//駆動輪の場合はエンジンの影響を考慮
	if (wheel.isDriveWheel && std::abs(m_throttleInput) > 0.001f) {
		float gearRatio = 1.0f; // 仮のギア比
		float engineAngularVelocity = (m_engine.currentRPM * 2.0f * 3.14159f / 60.0f) / gearRatio; // RPMをrad/sに変換

		//エンジン回転と車輪速度をブレンド
		float blendFactor = std::abs(m_throttleInput);
		targetAngularVelocity = targetAngularVelocity * (1.0f - blendFactor) + engineAngularVelocity * blendFactor;
	}

	//角速度のスムーズな変化
	float angularAcceleration = (targetAngularVelocity - wheel.angularVelocity) * 10.0f; // 仮の比例定数
	wheel.angularVelocity += angularAcceleration * deltaTime;

	//回転角度を更新
	wheel.rotationAngle += wheel.angularVelocity * deltaTime;

	//角度を0~2πの範囲に収める
	while (wheel.rotationAngle > XM_2PI) {
		wheel.rotationAngle -= XM_2PI;
	}
	while (wheel.rotationAngle < 0.0f) {
		wheel.rotationAngle += XM_2PI;
	}
}

void Vehicle::ApplyInertiaSteeringForce(Wheel& wheel, const Vector3& wheelVelocity, Vector3& tireForce) {
	//前輪のみに適用
	if (!wheel.isFrontWheel) {
		return;
	}

	//車両の前進速度を取得
	float forwardSpeed = m_velocity.Dot(GetForward());

	//前進慣性がある場合のみ適用
	if (forwardSpeed < 3.0f) {
		return;
	}

	//ステアリング角度がある場合
	if (std::abs(wheel.steerAngle) > 0.01f) {
		//ステアリング方向の単位ベクトルを計算
		Vector3 forward = GetForward();
		Vector3 right = GetRight();

		//y軸成分を0にして水平化
		forward.y = 0.0f;
		right.y = 0.0f;
		forward.Normalize();
		right.Normalize();

		float cosSteer = cosf(wheel.steerAngle);
		float sinSteer = sinf(wheel.steerAngle);
		Vector3 steerDirection = forward * cosSteer + right * sinSteer;

		//現在の進行方向
		Vector3 currentDirection = m_velocity;
		currentDirection.y = 0.0f;
		if (currentDirection.Length() > 0.1f) {
			currentDirection.Normalize();
		} else {
			return;
		}

		//ステアリング方向と現在の進行方向の角度差を計算
		Vector3 directionDiff = steerDirection - currentDirection;
		directionDiff.y = 0.0f;

		//慣性による前進力に比例したステアリング力を生産
		float steerForceMagnitude = forwardSpeed * std::abs(wheel.steerAngle) * 800.0f; // 仮の比例定数

		//速度に応じて効果を調整(高速時は効果を抑制)
		float speedFactor = std::max(0.3f, 1.0f - forwardSpeed * 0.08f);
		steerForceMagnitude *= speedFactor;

		//ステアリング力をタイヤ力に加算
		Vector3 steerForce = directionDiff * steerForceMagnitude;
		steerForce.y = 0.0f;

		//最大値を制限
		float maxSteerForce = wheel.maxGripForce * 0.2f; // 最大ステアリング力
		if (steerForce.Length() > maxSteerForce) {
			steerForce = steerForce * (maxSteerForce / steerForce.Length());
		}

		tireForce += steerForce;
	}
}

void Vehicle::ApplyDirectionTrackingForce() {
	float speed = m_velocity.Length();

	//速度が十分にある場合のみ適用
	if (speed < 5.0f) {
		return;
	}

	//車体の向き(水平成分)
	Vector3 carForward = GetForward();
	carForward.y = 0.0f;
	carForward.Normalize();

	Vector3 carRight = GetRight();
	carRight.y = 0.0f;
	carRight.Normalize();

	//実際の進行方向(水平成分)
	Vector3 velocityDirection = m_velocity;
	velocityDirection.y = 0.0f;
	if (velocityDirection.Length() < 0.1f) {
		return;
	}
	velocityDirection.Normalize();

	//車体方向と進行方向のズレ角度
	float forwardComponent = velocityDirection.Dot(carForward);
	float rightComponent = velocityDirection.Dot(carRight);

	//横方向のずれを修正する力
	Vector3 correctionForce = Vector3::ZERO;

	//横方向のずれがある場合
	if (std::abs(rightComponent) > 0.1f) {
		//横方向のズレを車体方向に修正する力
		float lateralCorrection = -rightComponent * speed * 500.0f; // 仮の比例定数

		//速度に応じて効果を調整
		float speedFactor = std::min(0.5f, speed / 30.0f);
		lateralCorrection *= speedFactor;

		correctionForce += carRight * lateralCorrection;
	}

	//前後方向の成分を車体前方向に向ける力
	if (forwardComponent > 0.0f) {
		float forwardCorrection = (0.8f - forwardComponent) * speed * 200.0f;
		correctionForce += carForward * forwardCorrection;
	}

	//修正力を車両に適用
	Vector3 totalCorrection = correctionForce / m_mass;
	m_acceleration += totalCorrection;

}

void Vehicle::CalculateAckermannSteering() {
	if (std::abs(m_steerInput) < 0.001f) {
		//ステアリング入力がない場合は両輪とも直進
		for (auto& wheel : m_wheels) {
			if (wheel.isFrontWheel) {
				wheel.steerAngle = 0.0f;
			}
		}
		return;
	}

	//基本ステアリング角度
	float baseSteerAngle = m_steerInput * m_maxSteerAngle;

	//アッカーマン角度の計算
	float tanBaseAngle = tan(std::abs(baseSteerAngle));
	if (tanBaseAngle < 0.001f) {
		//角度が非常に小さい場合
		for (auto& wheel : m_wheels) {
			if (wheel.isFrontWheel) {
				wheel.steerAngle = baseSteerAngle;
			}
		}

		return;
	}

	float turnRadius = m_wheelBase / tanBaseAngle;

	//内輪と外輪の角度を計算
	float innerRadius = turnRadius - m_trackWidth * 0.5f;
	float outerRadius = turnRadius + m_trackWidth * 0.5f;

	float innerAngle = atanf(m_wheelBase / innerRadius);
	float outerAngle = atanf(m_wheelBase / outerRadius);

	//左右どちらに曲がるか
	bool isTurningRight = (m_steerInput > 0.0f);

	//各車輪に角度を適用
	for (int i = 0; i < static_cast<int>(m_wheels.size()); i++) {
		if (m_wheels[i].isFrontWheel) {
			bool isLeftWheel = (i == 0);

			if (isTurningRight) {
				//右ターン
				if (isLeftWheel) {
					m_wheels[i].steerAngle = outerAngle;
				} else {
					m_wheels[i].steerAngle = innerAngle;
				}
			} else {
				//左ターン
				if (isLeftWheel) {
					m_wheels[i].steerAngle = -innerAngle;
				} else {
					m_wheels[i].steerAngle = -outerAngle;
				}
			}
		}
	}

}

float Vehicle::GetGroundHeight(const Vector3& position) const {
	return m_groundHeight;
}

Vector3 Vehicle::LocalToWorld(const Vector3& localPos) const {
	//オイラー角からの回転行列を適用
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(GetRotation().x, GetRotation().y, GetRotation().z);

	XMVECTOR localVec = XMVectorSet(localPos.x, localPos.y, localPos.z, 0.0f);
	XMVECTOR worldVec = XMVector3Transform(localVec, rotMatrix);

	Vector3 result;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), worldVec);

	return GetPosition() + result;

}

Vector3 Vehicle::WorldToLocal(const Vector3& worldPos) const {
	Vector3 relativePos = worldPos - GetPosition();

	//逆回転行列を適用
	XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(-GetRotation().x, -GetRotation().y, -GetRotation().z);

	XMVECTOR worldVec = XMVectorSet(relativePos.x, relativePos.y, relativePos.z, 0.0f);
	XMVECTOR localVec = XMVector3Transform(worldVec, rotMatrix);

	Vector3 result;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), localVec);
	return result;

}
