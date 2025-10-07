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
	, m_mass(75.0f)			//質量(kg)
	, m_waterDrag(2.5f)			//水の抵抗係数
	, m_propellerEfficiency(0.7f)	//プロペラ効率
	, m_maxTurnRate(0.7f)		//最大旋回速度(rad/s)
	, m_rollAmount(0.6f)		//ロール量
	, m_pitchAmount(0.2f)		//ピッチ量
	, m_yawRotation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_velocity(0.0f, 0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f, 0.0f)
	, m_angularVelocity(0.0f, 0.0f, 0.0f)
	, m_dragScalar(0.0f)
	, m_prevPosition(0.0f, 0.0f, 0.0f)
	, m_water(nullptr)
	, m_length(2.9f)			//ボートの長さ(m)
	, m_width(1.4f)				//ボートの幅(m)
	, m_height(0.5f)			//ボートの高さ(m)
	, m_verticalDamping(2.0f)	//垂直方向の減衰係数
	, m_buoyancyStiffness(1.2f) //浮力の剛性係数
	, m_restingWaterLevel(0.0f) //静止水面の高さ
	, m_waveFolllowStrength(1.5f) //波の追従距離
	, m_lastWaveForce(0.0f, 0.0f, 0.0f)
{
	//エンジントルクカーブ設定
	std::vector<TorquePoint> torqueCurve = {
		{ 800.0f, 40.0f },		//アイドリング
		{ 1500.0f, 65.0f },
		{ 2500.0f, 95.0f },
		{ 3500.0f, 120.0f },
		{ 4500.0f, 140.0f },
		{ 5500.0f, 145.0f },	//ピークトルク
		{ 6500.0f, 135.0f },
		{ 7500.0f, 115.0f },
		{ 8000.0f, 95.0f }		//レッドゾーン
	};

	m_engine.SetTorqueCurve(torqueCurve);

	//エンジンパラメータ設定
	m_engine.SetMaxPower(30.0f);		//最大出力(kw)
	m_engine.SetLoadSensitivity(0.9f);	//負荷感度

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
	//速度を取得
	float speed = GetSpeed();

	//抵抗スカラー更新
	UpdateDragScalar();

	//エンジンに負荷を設定
	m_engine.SetEngineLoad(speed, m_dragScalar);

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

	//ブレーキ力の計算
	Vector3 brakeForce = CalculateBrakeForce();

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
	totalForce += brakeForce;
//	totalForce += gravityForce;
//	totalForce += buoyancyForce;
//	totalForce += waveForce;

	//デバッグ表示
	std::cout << "ThrustForce: (" << thrustForce.x << ", " << thrustForce.y << ", " << thrustForce.z << ")" << std::endl;
	std::cout << "LateralForce: (" << lateralForce.x << ", " << lateralForce.y << ", " << lateralForce.z << ")" << std::endl;
	std::cout << "BrakeForce: (" << brakeForce.x << ", " << brakeForce.y << ", " << brakeForce.z << ")" << std::endl;
	std::cout << "DragForce: (" << dragForce.x << ", " << dragForce.y << ", " << dragForce.z << ")" << std::endl;
	std::cout << "GravityForce: (" << gravityForce.x << ", " << gravityForce.y << ", " << gravityForce.z << ")" << std::endl;
	std::cout << "BuoyancyForce: (" << buoyancyForce.x << ", " << buoyancyForce.y << ", " << buoyancyForce.z << ")" << std::endl;
	std::cout << "WaveForce: (" << waveForce.x << ", " << waveForce.y << ", " << waveForce.z << ")" << std::endl;

	//加速度の計算
	m_acceleration = totalForce / m_mass;

	//速度の更新
	m_velocity += m_acceleration * deltaTime;

	//位置更新
	m_position += m_velocity * deltaTime;
}

void Boat::UpdateDragScalar() {
	//速度に応じて抵抗スカラーを更新
	float speed = m_velocity.Length();

	//水に使っている高さを計算
	if (!m_water) {
		m_dragScalar = 0.0f;
		return;
	}
	float heightAboveWater = m_position.y + (m_height * 0.5f) - m_water->GetWaterHeight(m_position);

	//抵抗 = 0.5 * v^2 * Cd
	m_dragScalar = 0.5f * speed * speed * m_waterDrag;
}

Vector3 Boat::CalculateThrustForce() const {
	//推進力の計算

	//エンジン出力から推進力を計算
	float power = m_engine.GetCurrentPower(); //kw
	float currentSpeed = GetSpeed(); //m/s

	//推力(N) = 出力(W) * 効率 / 速度(m/s)
	//速度が低い場合は最大推力を制限
	float minSpeed = 0.1f; //最低速度(m/s)
	float effectiveSpeed = std::max(currentSpeed, minSpeed);

	float engineForce = (power * 1000.0f * m_propellerEfficiency) / effectiveSpeed; //N

	//スロットル入力を考慮
	engineForce *= m_throttleInput;

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

	//横方向の力を計算(
	float lateralForceMagnitude = -lateralSpeed * m_mass * 0.7f; //横速度に比例する力
	Vector3 lateralForce = right * lateralForceMagnitude;

	return lateralForce;
}

Vector3 Boat::CalculateBrakeForce() const {
	//ブレーキ力の計算
	if (m_velocity.Length() < 0.01f) return Vector3(0.0f, 0.0f, 0.0f); //ほぼ停止している場合はブレーキなし
	//ブレーキ力は速度と逆方向に働く
	Vector3 brakeDirection = -m_velocity;
	brakeDirection.Normalize();
	float speed = GetSpeed();
	float brakeForceMagnitude = m_brakeInput * m_mass * speed * 0.5f; //ブレーキ力の大きさ
	Vector3 brakeForce = brakeDirection * brakeForceMagnitude;
	return brakeForce;
}


Vector3 Boat::CalculateDragForce() const {
	//水の抵抗力の計算
	float speed = m_velocity.Length();

	//ほぼ停止している場合は抵抗なし
	if (speed < 0.01f) return Vector3(0.0f, 0.0f, 0.0f);

	//抵抗力は速度の2乗に比例し、速度と逆方向に働く
	Vector3 dragDirection = -m_velocity;
	dragDirection.Normalize();
	Vector3 dragForce = dragDirection * m_dragScalar;
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
		m_position + forward * (m_length * 0.4f) + right * (m_width * 0.4f),  // 前右
		m_position + forward * (m_length * 0.4f) - right * (m_width * 0.4f),  // 前左
		m_position - forward * (m_length * 0.4f) + right * (m_width * 0.4f),  // 後右
		m_position - forward * (m_length * 0.4f) - right * (m_width * 0.4f)   // 後左
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
	while (m_rotation.x < 0) m_rotation.x += XM_2PI;
	while (m_rotation.x >= XM_2PI) m_rotation.x -= XM_2PI;
	while (m_rotation.y < 0) m_rotation.y += XM_2PI;
	while (m_rotation.y >= XM_2PI) m_rotation.y -= XM_2PI;
	while (m_rotation.z < 0) m_rotation.z += XM_2PI;
	while (m_rotation.z >= XM_2PI) m_rotation.z -= XM_2PI;
}

void Boat::UpdateYaw(float deltaTime) {
	//ステアリングによる旋回
	if (std::abs(m_steeringInput) > 0.01f && std::abs(m_velocity.Length()) > 0.01f) {
		float turnAmount = m_maxTurnRate * m_steeringInput; //ステアリング入力に応じた旋回量(rad/s)
		m_angularVelocity.y = turnAmount * deltaTime; //今回フレームの旋回角度

		//速度に応じて旋回量を調整
		float speed = m_velocity.Length();
		//速度が早いほど旋回力を落とす(反比例)
		//速度が60m/sで旋回力が半分になるように調整
		float speedFactor = 1.0f - std::min(speed / 60.0f, 1.0f) * 0.5f;
		m_angularVelocity.y *= speedFactor;

		//速度が後退方向なら旋回方向を反転
		if (m_isReverse) {
			m_angularVelocity.y = -m_angularVelocity.y;
		}

		//クォータニオンを使って回転を適用
		Vector4 turnQuat = Vector4::FromAxisAngle(Vector3::UP, m_angularVelocity.y);
		m_yawRotation = turnQuat * m_yawRotation;
	}
}


Vector4 Boat::UpdateRoll(float deltaTime) {
	//ロール角を車体の方向と速度の変化に基づいて調整
	float targetRoll = 0.0f;

	if (std::abs(m_velocity.Length()) > 0.1f) {
		Vector3 right = m_yawRotation.RotateVector(Vector3::RIGHT);
		Vector3 normVel = m_velocity;
		normVel.Normalize();

		//速度が右か左か
		float lateralDot = normVel.Dot(right);
		targetRoll = m_rollAmount * lateralDot; //右に傾ける

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
		float speedFactor = std::min(GetSpeed() / 60.0f, 1.0f);
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
