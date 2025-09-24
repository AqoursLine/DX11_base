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
	, m_maxTurnRate(1.5)		//最大旋回速度(rad/s)
	, m_maxSpeed(80.0f)			//最大前進速度(m/s)
	, m_maxReverseSpeed(25.0f)	//最大後退速度(m/s)
	, m_rollAmount(1.0f)		//ロール量
	, m_velocity(0.0f, 0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f, 0.0f)
	, m_angularVelocity(0.0f, 0.0f, 0.0f)
	, m_prevPosition(0.0f, 0.0f, 0.0f)
	, m_water(nullptr)
	, m_length(4.0f)			//ボートの長さ(m)
	, m_width(2.5f)				//ボートの幅(m)
	, m_height(1.0f)			//ボートの高さ(m)
	, m_targetRoll(0.0f)
	, m_targetPitch(0.0f)
	, m_bobPhase(0.0f)
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

	return true;
}

void Boat::Update(double deltaTime) {
	//エンジン更新
	m_engine.Update(deltaTime);

	//前フレームの位置保存
	m_prevPosition = m_position;

	//deltaTimeをfloatに変換
	float dt = static_cast<float>(deltaTime);

	//物理計算
	UpdatePhysics(dt);

	//水面との相互作用更新
//	UpdateWaterInteraction(dt);

	//姿勢制御
	UpdateRotation(dt);
//	UpdateRoll(dt);
//	UpdatePitch(dt);
//	UpdateBobbing(dt);

	//rotationを更新
	m_rotation = m_quaternion.ToEuler();
}

void Boat::UpdatePhysics(float deltaTime) {
	//合計
	Vector3 totalForce(0.0f, 0.0f, 0.0f);

	//推進力の計算
	Vector3 thrustForce = CalculateThrustForce();

	//水の抵抗力の計算
	Vector3 dragForce = CalculateDragForce();

	//重力
	Vector3 gravityForce(0.0f, -m_mass * GRAVITY, 0.0f);

	//浮力
	Vector3 buoyancyForce = CalculateBuoyancyForce();

	//合計力の計算
	totalForce += thrustForce;
	totalForce += dragForce;
	totalForce += gravityForce;
	totalForce += buoyancyForce;

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
	Vector3 forwardDir = GetForwardQ();
	Vector3 thrustForce = forwardDir * engineForce;

	return thrustForce;
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

	//ボートの底面のY座標を計算
	float boatBottomY = m_position.y - (m_height * 0.5f);
	//水面の高さを取得
	float waterHeight = m_water->GetWaterHeight(m_position);
	//水面下にあるか判定
	if (boatBottomY >= waterHeight) {
		//水面上にある場合、浮力なし
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	//水面下にある場合、浮力を計算
	float submergedDepth = waterHeight - boatBottomY; //水没深さ
	submergedDepth = std::min(submergedDepth, m_height); //最大でもボートの高さまで
	float submergedVolume = m_length * m_width * submergedDepth; //水没体積(m^3)
	float waterDensity = 1000.0f; //水の密度(kg/m^3)
	float buoyancyMagnitude = waterDensity * GRAVITY * submergedVolume; //浮力の大きさ(N)
	return Vector3(0.0f, buoyancyMagnitude, 0.0f);
}

void Boat::UpdateWaterInteraction(float deltaTime) {
	if (!m_water) return;

	//航跡の更新
	Vector3 movement = m_position - m_prevPosition;
	float moveDistance = movement.Length();
	float currentSpeed = GetSpeed();

	if (moveDistance > 0.1f && currentSpeed > 2.0f) {
		//前進・後退の判定
		Vector3 forwardDir = GetForwardQ();
		bool movingForward = m_velocity.Dot(forwardDir) > 0.0f;

		//後退時は航跡の強度を下げる
		float wakeIntensity = movingForward ? 1.0f : 0.4f;

		m_water->UpdateBoatWake(m_position, m_prevPosition, currentSpeed, m_width);
	}

	//高速時の追加波紋効果
	Vector3 forwardDir = GetForwardQ();
	bool movingForward = m_velocity.Dot(forwardDir) > 0.0f;

	if (movingForward && currentSpeed > 15.0f) {
		Vector3 rightDir = GetRightQ();
		float waveIntensity = std::min(currentSpeed / 50.0f, 2.0f); //速度に応じて強度調整

		//左右に波紋を追加
		Vector3 leftPos = m_position - rightDir * (m_width * 0.8f);
		Vector3 rightPos = m_position + rightDir * (m_width * 0.8f);

		static float waveTimer = 0.0f;
		waveTimer += deltaTime;

		if (waveTimer > 0.2f) {
			m_water->AddRipple(leftPos, waveIntensity * 0.4f, 1.5f, 6.0f);
			m_water->AddRipple(rightPos, waveIntensity * 0.4f, 1.5f, 6.0f);
			waveTimer = 0.0f;
		}
	}
}

void Boat::UpdateRotation(float deltaTime) {
	//ステアリングによる旋回
	if (std::abs(m_steeringInput) > 0.01f) {
		float turnAmount = m_maxTurnRate * m_steeringInput; //ステアリング入力に応じた旋回量(rad/s)
		float turnAngle = turnAmount * deltaTime; //今回フレームの旋回角度

		//クォータニオンを使って回転を適用
		Vector4 turnQuat = Vector4::FromAxisAngle(GetUpQ(), turnAngle);
		m_quaternion = turnQuat * m_quaternion;
		m_quaternion.Normalize();
	} else {
	}
}

void Boat::ApplyForces(float deltaTime) {
	//この関数は将来的に外部からの力を適用する際に使用予定
	//現在は物理計算が完結しているため未使用
}

void Boat::UpdateRoll(float deltaTime) {
}

void Boat::UpdatePitch(float deltaTime) {
}

void Boat::UpdateBobbing(float deltaTime) {
	if (!m_water) return;
}
