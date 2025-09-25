#include "boat.h"
#include "water.h"
#include <algorithm>

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "water.h"

Boat::Boat()
	: m_throttleInput(0.0f)
	, m_steeringInput(0.0f)
	, m_brakeInput(0.0f)
	, m_isReverse(false)
	, m_mass(1500.0f)			//質量(kg)
	, m_waterDrag(1.0f)			//水の抵抗係数
	, m_maxTurnRate(0.7)		//最大旋回速度(rad/s)
	, m_maxSpeed(60.0f)			//最大前進速度(m/s)
	, m_maxReverseSpeed(25.0f)	//最大後退速度(m/s)
	, m_rollAmount(0.6f)		//ロール量
	, m_pitchAmount(0.3f)		//ピッチ量
	, m_yawRotation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_velocity(0.0f, 0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f, 0.0f)
	, m_angularVelocity(0.0f, 0.0f, 0.0f)
	, m_prevPosition(0.0f, 0.0f, 0.0f)
	, m_water(nullptr)
	, m_length(4.0f)			//ボートの長さ(m)
	, m_width(2.5f)				//ボートの幅(m)
	, m_height(1.0f)			//ボートの高さ(m)
	, m_verticalDamping(2.0f)	//垂直方向の減衰係数
	, m_buoyancyStiffness(1.2f) //浮力の剛性係数
	, m_restingWaterLevel(0.0f) //静止水面の高さ
	, m_waveFolllowStrength(1.5f) //波の追従距離
	, m_lastWaveForce(0.0f, 0.0f, 0.0f)
{
	m_engine.SetMaxPower(1500.0f);	//最大出力(W)
	m_engine.SetAcceleration(1.8f);	//加速度
	m_engine.SetDeceleration(2.5f);	//減速度
}

void Boat::SetThrottle(float throttle)
{
	m_throttleInput = std::clamp(throttle, 0.0f, 1.0f);
	m_engine.SetThrottle(m_throttleInput);

}

void Boat::SetSteering(float steering) {
	m_steeringInput = std::clamp(steering, -1.0f, 1.0f);
}

void Boat::SetBrake(float brake) {
	m_brakeInput = std::clamp(brake, 0.0f, 1.0f);
}

void Boat::SetReverse(bool isReverse) {
	m_isReverse = isReverse;
}

bool Boat::Initialize() {
	m_prevPosition = m_position;
	m_water = SYSTEM.GetManager()->GetScene()->GetGameObject<Water>();

	//静止水面の高さを設定
	if (m_water) {
		m_restingWaterLevel = m_water->GetWaterHeight(m_position);
	} else {
		m_restingWaterLevel = 0.0f;
	}

	return true;
}

void Boat::Update(double deltaTime) {
	//エンジン更新
	m_engine.Update(deltaTime);

	//前フレームの位置保存
	m_prevPosition = m_position;

	//deltaTimeをfloatに変換
	float dt = static_cast<float>(deltaTime);

	//姿勢制御
	UpdateRotation(dt);

	//物理計算
	UpdatePhysics(dt);

	//水面との相互作用更新
	UpdateWaterInteraction(dt);
}

void Boat::UpdatePhysics(float deltaTime) {
	//合計
	Vector3 totalForce(0.0f, 0.0f, 0.0f);

	//推進力の計算
	Vector3 thrustForce = CalculateThrustForce();

	//横方向の力の計算（スリップ防止用）
	Vector3 lateralForce = CalculateLateralForce();

	//水の抵抗力の計算
	Vector3 dragForce = CalculateDragForce();

	//重力
	Vector3 gravityForce(0.0f, -m_mass * GRAVITY, 0.0f);

	//浮力
	Vector3 buoyancyForce = CalculateBuoyancyForce();

	//波による追加の力
	Vector3 waveForce = CalculateWaveForce();

	//合計力の計算
	totalForce += thrustForce;
	totalForce += lateralForce;
	totalForce += dragForce;
//	totalForce += gravityForce;
//	totalForce += buoyancyForce;
//	totalForce += waveForce;

	//デバッグ表示
	std::cout << "ThrustForce: (" << thrustForce.x << ", " << thrustForce.y << ", " << thrustForce.z << ")" << std::endl;
	std::cout << "LateralForce: (" << lateralForce.x << ", " << lateralForce.y << ", " << lateralForce.z << ")" << std::endl;
	std::cout << "DragForce: (" << dragForce.x << ", " << dragForce.y << ", " << dragForce.z << ")" << std::endl;
	std::cout << "GravityForce: (" << gravityForce.x << ", " << gravityForce.y << ", " << gravityForce.z << ")" << std::endl;
	std::cout << "BuoyancyForce: (" << buoyancyForce.x << ", " << buoyancyForce.y << ", " << buoyancyForce.z << ")" << std::endl;
	std::cout << "WaveForce: (" << waveForce.x << ", " << waveForce.y << ", " << waveForce.z << ")" << std::endl;

	//加速度の計算
	m_acceleration = totalForce / m_mass;

	//速度の更新
	m_velocity += m_acceleration * deltaTime;

	//速度制限
	float speed = m_velocity.Length();
	if (m_isReverse) {
		if (speed > m_maxReverseSpeed) {
			m_velocity = m_velocity * (m_maxReverseSpeed / speed);
		}
	} else {
		if (speed > m_maxSpeed) {
			m_velocity = m_velocity * (m_maxSpeed / speed);
		}
	}

	//位置更新
	m_position += m_velocity * deltaTime;
}

Vector3 Boat::CalculateThrustForce() const {
	//推進力の計算
	float enginePowerRatio = m_engine.GetCurrentPower() / m_engine.GetMaxPower(); //単純化のため、速度に比例する力とする
	float baseForce = 18000.0f; //最低限の推進力
	float engineForce = baseForce * enginePowerRatio; //エンジン出力に応じた推進力

	//リバースギア時は力を反転
	if (m_isReverse) {
		engineForce *= -0.6f; //リバースは前進の60%の力
	}

	//前進方向の力quaternion版
	Vector3 forwardDir = m_yawRotation.RotateVector(Vector3::FORWARD);
	Vector3 thrustForce = forwardDir * engineForce;

	return thrustForce;
}

Vector3 Boat::CalculateLateralForce() const {
	//横方向の力の計算（スリップ防止用）
	if (m_velocity.Length() < 0.01f) return Vector3(0.0f, 0.0f, 0.0f); //ほぼ停止している場合は横力なし

	//横方向の速度成分を取得
	Vector3 right = m_yawRotation.RotateVector(Vector3::RIGHT); //右方向ベクトル

	float lateralSpeed = m_velocity.Dot(right);

	//横方向の力を計算
	float lateralForceMagnitude = -lateralSpeed * m_mass * 0.5f; //横速度に比例する力
	Vector3 lateralForce = right * lateralForceMagnitude;

	return lateralForce;
}

Vector3 Boat::CalculateDragForce() const {
	//水の抵抗力の計算
	float speed = m_velocity.Length();
	if (speed < 0.01f) return Vector3(0.0f, 0.0f, 0.0f); //ほぼ停止している場合は抵抗なし
	//抵抗力は速度の2乗に比例し、速度と逆方向に働く
	float dragMagnitude = m_waterDrag * speed * speed;
	Vector3 dragDirection = -m_velocity;
	dragDirection.Normalize();
	Vector3 dragForce = dragDirection * dragMagnitude;
	return dragForce;
}

Vector3 Boat::CalculateBuoyancyForce() const {
	//浮力の計算
	if (!m_water) return Vector3(0.0f, 0.0f, 0.0f);

	//ボートの向きを取得
	Vector3 forward = GetForwardQ();
	Vector3 right = GetRightQ();

	//ボートの4隅の位置を計算
	Vector3 corners[4] = {
		m_position + forward * (m_length * 0.4) + right * (m_width  * 0.4),  // 前右
		m_position + forward * (m_length * 0.4) - right * (m_width * 0.4),  // 前左
		m_position - forward * (m_length * 0.4) + right * (m_width * 0.4),  // 後右
		m_position - forward * (m_length * 0.4) - right * (m_width * 0.4)   // 後左
	};

	float totalBuoyancy = 0.0f;

	for (int i = 0; i < 4; i++) {
		float cornerBottomY = corners[i].y - (m_height * 0.5f);
		float waterHeight = m_water->GetWaterHeight(corners[i]);

		if (cornerBottomY < waterHeight) {
			float submergedDepth = waterHeight - cornerBottomY;
			submergedDepth = std::min(submergedDepth, m_height); //最大でボートの高さまで

			//デッドゾーン
			const float deadZone = 0.02f;
			if (submergedDepth > deadZone) {
				submergedDepth -= deadZone;

				//各角の浮力を計算
				float cornerVolume = (m_length * m_width * 0.25f) * submergedDepth;
				totalBuoyancy += WATER_DENSITY * GRAVITY * cornerVolume;
			}
		}
	}

	//浮力係数とスムージングを適用
	totalBuoyancy *= m_buoyancyStiffness;

	//最大浮力制限
	const float maxBuoyancy = m_mass * GRAVITY * 1.0001f;
	totalBuoyancy = std::min(totalBuoyancy, maxBuoyancy);

	return Vector3(0.0f, totalBuoyancy, 0.0f);
}

Vector3 Boat::CalculateWaveForce() {
	if (!m_water) return Vector3(0.0f, 0.0f, 0.0f);

	//現在の水面高度を取得
	float currentWaterHeight = m_water->GetWaterHeight(m_position);
	float boatBottomY = m_position.y - (m_height * 0.5f);

	//波による高度差を計算
	float waveHeightDiff = currentWaterHeight - m_restingWaterLevel;

	//ボートが水面にある場合のみ波の影響を受ける
	if (boatBottomY < currentWaterHeight + 0.5f) {
		//波の高度差に応じて上向きの力を生成
		float waveForce = waveHeightDiff * m_waveFolllowStrength * m_mass * GRAVITY;

		//波の変化率も考慮
		float waveVelocity = (waveHeightDiff - m_lastWaveForce.y) / (1.0f / 60.0f); //仮に60FPSとして計算
		waveForce += waveVelocity * m_mass * 0.5f; //波の変化に応じた追加力

		Vector3 result(0.0f, waveForce, 0.0f);
		m_lastWaveForce = Vector3(0.0f, waveHeightDiff, 0.0f);

		return result;
	}

	return Vector3(0.0f, 0.0f, 0.0f);
}

void Boat::UpdateWaterInteraction(float deltaTime) {
	if (!m_water) return;

	//水面の高さに基づいてボートのy位置を調整
	float waterHeight = m_water->GetWaterHeight(m_position);
	float targetY = waterHeight + (m_height * 0.3f); //ボートの底が水面に接するように調整

	m_position.y = targetY;
}

void Boat::ApplyForces(float deltaTime) {
	//この関数は将来的に外部からの力を適用する際に使用予定
	//現在は物理計算が完結しているため未使用
}

void Boat::UpdateRotation(float deltaTime) {
	UpdateYaw(deltaTime);
	Vector4 rollRotation = UpdateRoll(deltaTime);
	Vector4 pitchRotation = UpdatePitch(deltaTime);
	UpdateBobbing(deltaTime);

	//最終的なクォータニオンを計算
	m_quaternion = m_yawRotation;
	m_quaternion = rollRotation * m_quaternion;
	m_quaternion = pitchRotation * m_quaternion;
	m_quaternion.Normalize();

	//rotationを更新
	m_rotation = m_quaternion.ToEuler();

	//回転範囲を0~2πに制限
	if (m_rotation.x < 0.0f) {
		m_rotation.x += XM_2PI;
	} else if (m_rotation.x >= XM_2PI) {
		m_rotation.x -= XM_2PI;
	}
	if (m_rotation.y < 0.0f) {
		m_rotation.y += XM_2PI;
	} else if (m_rotation.y >= XM_2PI) {
		m_rotation.y -= XM_2PI;
	}
	if (m_rotation.z < 0.0f) {
		m_rotation.z += XM_2PI;
	} else if (m_rotation.z >= XM_2PI) {
		m_rotation.z -= XM_2PI;
	}
}

void Boat::UpdateYaw(float deltaTime) {
	//ステアリングによる旋回
	if (std::abs(m_steeringInput) > 0.01f && std::abs(m_velocity.Length()) > 0.01f) {
		float turnAmount = m_maxTurnRate * m_steeringInput; //ステアリング入力に応じた旋回量(rad/s)
		float turnAngle = turnAmount * deltaTime; //今回フレームの旋回角度

		//速度に応じて旋回量を調整
		float speed = m_velocity.Length();
		float speedFactor = std::min(speed / m_maxSpeed, 1.0f); //速度に応じた係数(0.0~1.0)
		speedFactor = std::max(speedFactor, 0.2f); //最低でも20%の旋回力は残す
		turnAngle *= speedFactor;

		//速度が後退方向なら旋回方向を反転
		if (m_isReverse) {
			turnAngle = -turnAngle;
		}

		//クォータニオンを使って回転を適用
		Vector4 turnQuat = Vector4::FromAxisAngle(Vector3::UP, turnAngle);
		m_yawRotation = turnQuat * m_yawRotation;
	}
}


Vector4 Boat::UpdateRoll(float deltaTime) {
	//ロールの目標値を速度とステアリングに基づいて計算
	float targetRoll = 0.0f;
	if (std::abs(m_velocity.Length()) > 0.1f) {
		float speedFactor = std::min(m_velocity.Length() / m_maxSpeed, 1.0f);
		targetRoll = -m_steeringInput * m_rollAmount * speedFactor; //ステアリングに応じたロール量
	}

	//クォータニオンを作成
	Vector3 forward = m_yawRotation.RotateVector(Vector3::FORWARD);
	Vector4 rollQuat = Vector4::FromAxisAngle(forward, targetRoll);

	return rollQuat;
}

Vector4 Boat::UpdatePitch(float deltaTime) {
	//ピッチを速度に基づいて上げる
	float targetPitch = 0.0f;
	if (m_velocity.Length() > 0.1f) {
		//速度が早いほど前方のピッチを上げる
		float speedFactor = std::min(m_velocity.Length() / m_maxSpeed, 1.0f);
		targetPitch = m_pitchAmount * -speedFactor; //上に傾ける
	}

	//クォータニオンを作成
	Vector3 right = m_yawRotation.RotateVector(Vector3::RIGHT);
	Vector4 pitchQuat = Vector4::FromAxisAngle(right, targetPitch);
	return pitchQuat;
}

void Boat::UpdateBobbing(float deltaTime) {
	if (!m_water) return;
}
