#include "boat.h"
#include "water.h"
#include <algorithm>

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "water.h"
#include "raceManager.h"
#include "splashParticle.h"

Boat::Boat()
	: m_throttleInput(0.0f)	//スロットル入力(0.0f ~ 1.0f)
	, m_steeringInput(0.0f)	//ステアリング入力(-1.0f ~ 1.0f)
	, m_brakeInput(0.0f)	//ブレーキ入力(0.0f ~ 1.0f)
	, m_isReverse(false)	//リバースギアフラグ

	, m_mass(75.0f)					//質量(kg)
	, m_waterDrag(2.5f)				//水の抵抗係数
	, m_propellerEfficiency(0.5f)	//プロペラ効率
	, m_maxTurnRate(0.7f)			//最大旋回速度(rad/s)

	, m_verticalDamping(2.0f)	//垂直方向の減衰係数
	, m_restingWaterLevel(0.0f) //静止水面の高さ
	, m_waveForceScale(5.0f)	//波の力スケール
	, m_isInWater(false)		//水中フラグ

	, m_velocity(0.0f, 0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f, 0.0f)
	, m_angularVelocity(0.0f, 0.0f, 0.0f)
	, m_dragScalar(0.0f)

	, m_prevPosition(0.0f, 0.0f, 0.0f)

	, m_water(nullptr)
	, m_splashEffect(nullptr)
	, m_splashTimer(0.0f)

	, m_length(2.9f)			//ボートの長さ(m)
	, m_width(1.4f)				//ボートの幅(m)
	, m_height(0.8f)			//ボートの高さ(m)
	, m_wallPenetrationDepth(0.0f, 0.0f)	//壁へのめり込み深さ

	, m_rollAmount(0.6f)		//ロール量
	, m_pitchAmount(0.15f)		//ピッチ量
	, m_yawRotation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_pitchRotation(0.0f, 0.0f, 0.0f, 1.0f)
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
	m_engine.SetMaxPower(35.0f);		//最大出力(kw)
	m_engine.SetLoadSensitivity(1.0f);	//負荷感度

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
	m_water = m_scene->GetGameObject<Water>();

	//静止水面の高さを設定
	if (m_water) {
		m_restingWaterLevel = m_water->GetWaterHeight(m_position);
	} else {
		m_restingWaterLevel = 0.0f;
	}

	m_position.y = m_restingWaterLevel - 0.2f;
	m_prevPosition = m_position;

	// 水しぶきパーティクル取得
	m_splashEffect = m_scene->GetGameObject<SplashParticle>();

	return true;
}

void Boat::Update(double deltaTime) {
	//速度を取得
	float speed = GetSpeed();

	//水平速度を取得
	Vector3 horizontalVelocity = Vector3(m_velocity.x, 0.0f, m_velocity.z);
	float horizontalSpeed = horizontalVelocity.Length();

	//抵抗スカラー更新
	UpdateDragScalar(horizontalSpeed);

	//エンジンに負荷を設定
	m_engine.SetEngineLoad(speed, m_dragScalar);

	//エンジン更新
	m_engine.Update(deltaTime);

	//前フレームの位置保存
	m_prevPosition = m_position;

	//deltaTimeをfloatに変換
	float dt = static_cast<float>(deltaTime);

	//物理計算
	UpdatePhysics(dt, speed, horizontalSpeed);

	//姿勢制御
	UpdateRotation(dt, horizontalSpeed);

	//水面との相互作用更新
	UpdateWaterInteraction(dt);

	//4隅の座標更新
	UpdateCorners();
}

Vector2 Boat::GetSceneBoundsMin() const {
	//シーンの最小座標を取得
	return m_scene->GetBoundsMin();
}

Vector2 Boat::GetSceneBoundsMax() const {
	//シーンの最大座標を取得
	return m_scene->GetBoundsMax();
}


void Boat::UpdatePhysics(float deltaTime, float speed, float horizontalSpeed) {
	//合計
	Vector3 totalForce(0.0f, 0.0f, 0.0f);

	//推進力の計算
	Vector3 thrustForce = CalculateThrustForce();

	//横方向の力の計算（スリップ防止用）
	Vector3 lateralForce = CalculateLateralForce();

	//ブレーキ力の計算
	Vector3 brakeForce = CalculateBrakeForce(horizontalSpeed);

	//水の抵抗力の計算
	Vector3 dragForce = CalculateDragForce(speed);

	//重力
	Vector3 gravityForce(0.0f, -m_mass * GRAVITY, 0.0f);

	//浮力
	Vector3 buoyancyForce = CalculateBuoyancyForce(deltaTime);

	//波による追加の力
	Vector3 waveForce = CalculateWaveForce();

	//壁との衝突力
	Vector3 wallCollisionForce = CalculateWallCollisionForce();

	//合計力の計算
	totalForce += thrustForce;
	totalForce += lateralForce;
	totalForce += dragForce;
	totalForce += brakeForce;
	totalForce += gravityForce;
	totalForce += buoyancyForce;
	totalForce += waveForce;
	totalForce += wallCollisionForce;

	//壁めり込み補正
	if (wallCollisionForce.Length() > 0.01f) {
		m_position.x += m_wallPenetrationDepth.x;
		m_position.z += m_wallPenetrationDepth.y;
	}

	//加速度の計算
	m_acceleration = totalForce / m_mass;

	//速度の更新
	m_velocity += m_acceleration * deltaTime;

	//位置更新
	m_position += m_velocity * deltaTime;

}

void Boat::UpdateDragScalar(float horizontalSpeed) {
	//抵抗 = 0.5 * v^2 * Cd
	m_dragScalar = 0.5f * horizontalSpeed * horizontalSpeed * m_waterDrag;
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

	if (std::abs(lateralSpeed) < 0.001f) {
		return Vector3(0.0f, 0.0f, 0.0f); //横速度がほぼゼロなら横力なし
	}

	//横方向の力を計算(
	float lateralForceMagnitude = -lateralSpeed * m_mass * 0.7f; //横速度に比例する力
	Vector3 lateralForce = right * lateralForceMagnitude;

	return lateralForce;
}

Vector3 Boat::CalculateBrakeForce(float horizontalSpeed) const {
	//ブレーキ力の計算
	if (m_velocity.Length() < 0.01f) return Vector3(0.0f, 0.0f, 0.0f); //ほぼ停止している場合はブレーキなし
	//ブレーキ力は速度と逆方向に働く
	Vector3 brakeDirection = -m_velocity;
	brakeDirection.Normalize();
	float brakeForceMagnitude = m_brakeInput * m_mass * horizontalSpeed * 0.5f; //ブレーキ力の大きさ
	Vector3 brakeForce = brakeDirection * brakeForceMagnitude;
	return brakeForce;
}


Vector3 Boat::CalculateDragForce(float speed) const {

	//ほぼ停止している場合は抵抗なし
	if (speed < 0.01f) return Vector3(0.0f, 0.0f, 0.0f);

	//抵抗力は速度の2乗に比例し、速度と逆方向に働く
	Vector3 dragDirection = -m_velocity;
	dragDirection.y = 0.0f; //水平成分のみ
	float horizontalSpeed = dragDirection.Length();

	if (horizontalSpeed > 0.01f) {
		dragDirection = dragDirection / horizontalSpeed; //正規化
		Vector3 dragForce = dragDirection * m_dragScalar;

		//垂直方向の減衰を追加
		dragForce.y = -m_velocity.y * m_verticalDamping * m_mass;

		return dragForce;
	}
	return Vector3(0.0f, 0.0f, 0.0f);
}

Vector3 Boat::CalculateBuoyancyForce(float deltaTime) {
	//浮力の計算
	if (!m_water) return Vector3(0.0f, 0.0f, 0.0f);

	//現在の水面高度を取得
	float currentWaterHeight = m_water->GetWaterHeight(m_position + m_velocity * deltaTime);

	//ボートの底面のy座標
	float boatBottomY = m_position.y - (m_height * 0.5f);

	//ボートが水に浸かっているか判定
	if (boatBottomY < currentWaterHeight) {
		//直前に水に浸かっていなかった場合
		if (!m_isInWater) {
//			m_water->AddRipple(m_position);
		}

		m_isInWater = true;

		//浸かっている深さを計算
		float submergedDepth = currentWaterHeight - boatBottomY;

		float buoyancyMagnitude = submergedDepth * m_mass * GRAVITY * 4.0f; //浮力の大きさ（調整可能）

		return Vector3(0.0f, buoyancyMagnitude, 0.0f);
	}

	m_isInWater = false;
	
	return Vector3(0.0f, 0.0f, 0.0f);
}

Vector3 Boat::CalculateWaveForce() {
	if (!m_water) return Vector3(0.0f, 0.0f, 0.0f);

	if (!m_isInWater) return Vector3(0.0f, 0.0f, 0.0f);

	//波の法線ベクトルを取得
	Vector3 waveNormal = m_water->GetWaterNormal(m_position);

	//水平方向だけにする
	Vector3 horizontalForce = waveNormal * m_mass * m_waveForceScale;

	horizontalForce.y = 0.0f;

	return horizontalForce;
}

Vector3 Boat::CalculateWallCollisionForce() {
	//回転を考慮したボートの4隅の位置を計算
	if (!m_scene) return Vector3(0.0f, 0.0f, 0.0f);

	//シーンの境界を取得
	Vector2 sceneMin = GetSceneBoundsMin();
	Vector2 sceneMax = GetSceneBoundsMax();

	//衝突力
	Vector3 collisionForce(0.0f, 0.0f, 0.0f);

	//めり込み深さリセット
	m_wallPenetrationDepth = Vector2(0.0f, 0.0f);

	//X軸方向の衝突
	for (int i = 0; i < 4; i++) {
		if (m_corners[i].x < sceneMin.x) {
			m_wallPenetrationDepth.x += sceneMin.x - m_corners[i].x;
		} else if (m_corners[i].x > sceneMax.x) {
			m_wallPenetrationDepth.x -= m_corners[i].x - sceneMax.x;
		}
	}

	//Z軸方向の衝突
	for (int i = 0; i < 4; i++) {
		if (m_corners[i].z < sceneMin.y) {
			m_wallPenetrationDepth.y += sceneMin.y - m_corners[i].z;
		} else if (m_corners[i].z > sceneMax.y) {
			m_wallPenetrationDepth.y -= m_corners[i].z - sceneMax.y;
		}
	}

	//衝突力を計算
	if (std::abs(m_wallPenetrationDepth.x) > 0.01f) {
		collisionForce.x = m_wallPenetrationDepth.x * m_mass * 10.0f; //めり込み深さに比例した力
	}
	if (std::abs(m_wallPenetrationDepth.y) > 0.01f) {
		collisionForce.z = m_wallPenetrationDepth.y * m_mass * 10.0f; //めり込み深さに比例した力
	}

	//摩擦力を追加
	if (collisionForce.Length() > 0.01f) {
		Vector3 horizontalVelocity = Vector3(m_velocity.x, 0.0f, m_velocity.z);
		if (horizontalVelocity.Length() > 0.01f) {
			horizontalVelocity.Normalize();
			Vector3 frictionForce = -horizontalVelocity * m_mass * 5.0f; //摩擦力
			collisionForce += frictionForce;
		}
	}
	return collisionForce;
}

void Boat::UpdateWaterInteraction(float deltaTime) {
	if (!m_water) return;

	if (GetSpeed() > 10.0f) {
		m_splashTimer += deltaTime;

		if (m_splashTimer >= 0.5f) {
			m_water->AddRipple(m_position, 0.5f, 2.0f);

			m_splashTimer = 0.0f;
		}

		//水しぶきエフェクト生成
		if (m_splashEffect) {
			Vector3 forward = GetForwardQ();
			Vector3 splashPos = m_position - forward * (m_length * 0.5f);
			splashPos.y -= m_height* 0.3f;

			m_splashEffect->EmitOneShot(splashPos);
		}
	}
}

void Boat::ApplyForces(float deltaTime) {
	//この関数は将来的に外部からの力を適用する際に使用予定
	//現在は物理計算が完結しているため未使用
}

void Boat::UpdateRotation(float deltaTime, float horizontalSpeed) {
	UpdateYaw(deltaTime, horizontalSpeed);
	Vector4 rollRotation = UpdateRoll(deltaTime);
	UpdatePitch(deltaTime, horizontalSpeed);

	//最終的なクォータニオンを計算
	m_quaternion = m_yawRotation;
	m_quaternion = rollRotation * m_quaternion;
	m_quaternion = m_pitchRotation * m_quaternion;
	m_quaternion.Normalize();

	//rotationを更新
	m_rotation = m_quaternion.ToEuler();

	//回転範囲を0~2πに制限
	m_rotation.x = std::fmod(m_rotation.x, XM_2PI);
	m_rotation.y = std::fmod(m_rotation.y, XM_2PI);
	m_rotation.z = std::fmod(m_rotation.z, XM_2PI);
	if (m_rotation.x < 0) m_rotation.x += XM_2PI;
	if (m_rotation.y < 0) m_rotation.y += XM_2PI;
	if (m_rotation.z < 0) m_rotation.z += XM_2PI;
}

void Boat::UpdateYaw(float deltaTime, float horizontalSpeed) {
	//ステアリングによる旋回
	if (std::abs(m_steeringInput) > 0.01f && std::abs(m_velocity.Length()) > 0.01f) {
		float turnAmount = m_maxTurnRate * m_steeringInput; //ステアリング入力に応じた旋回量(rad/s)
		m_angularVelocity.y = turnAmount * deltaTime; //今回フレームの旋回角度

		//速度が早いほど旋回力を落とす(反比例)
		//速度が60m/sで旋回力が半分になるように調整
		float speedFactor = 1.0f - std::min(horizontalSpeed / 60.0f, 1.0f) * 0.5f;
		m_angularVelocity.y *= speedFactor;

		//速度が後退方向なら旋回方向を反転
		if (m_isReverse) {
			m_angularVelocity.y = -m_angularVelocity.y;
		}

		//クォータニオンを使って回転を適用
		Vector4 turnQuat = Vector4::FromAxisAngle(Vector3::UP, m_angularVelocity.y);
		m_yawRotation = turnQuat * m_yawRotation;
		m_yawRotation.Normalize();
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

void Boat::UpdatePitch(float deltaTime, float horizontalSpeed) {
	//ピッチを速度に基づいて上げる
	float targetPitch = 0.0f;
	if (horizontalSpeed > 0.1f) {
		//速度が早いほど前方のピッチを上げる
		float speedFactor = std::min(horizontalSpeed / 30.0f, 1.0f);
		targetPitch = m_pitchAmount * -speedFactor; //上に傾ける
	}

	//クォータニオンを作成
	Vector3 right = m_yawRotation.RotateVector(Vector3::RIGHT);
	m_pitchRotation = Vector4::FromAxisAngle(right, targetPitch);
}

void Boat::UpdateCorners() {
	//回転を考慮したボートの4隅の位置を計算
	if (!m_scene) return;
	//前方と右方向のベクトルを取得
	Vector3 forward = m_yawRotation.RotateVector(Vector3::FORWARD);
	Vector3 right = m_yawRotation.RotateVector(Vector3::RIGHT);
	//ボートの4隅の位置を計算
	m_corners[0] = m_position + forward * (m_length * 0.5f) + right * (m_width * 0.5f);  // 前右
	m_corners[1] = m_position + forward * (m_length * 0.5f) - right * (m_width * 0.5f);  // 前左
	m_corners[2] = m_position - forward * (m_length * 0.5f) + right * (m_width * 0.5f);  // 後右
	m_corners[3] = m_position - forward * (m_length * 0.5f) - right * (m_width * 0.5f);   // 後左
}
