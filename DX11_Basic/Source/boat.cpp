#include "boat.h"
#include "water.h"
#include <algorithm>

Boat::Boat()
	: m_throttleInput(0.0f)
	, m_steeringInput(0.0f)
	, m_brakeInput(0.0f)
	, m_isReverse(false)
	, m_mass(500.0f)			//質量(kg)
	, m_waterDrag(0.8f)			//水の抵抗係数
	, m_maxTurnRate(1.5)		//最大旋回速度(rad/s)
	, m_maxSpeed(80.0f)			//最大前進速度(m/s)
	, m_maxReverseSpeed(25.0f)	//最大後退速度(m/s)
	, m_rollAmount(1.0f)		//ロール量
	, m_velocity(0.0f, 0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f, 0.0f)
	, m_angularVelocity(0.0f, 0.0f, 0.0f)
	, m_prevPosition(0.0f, 0.0f, 0.0f)
	, m_water(nullptr)
	, m_length(8.0f)			//ボートの長さ(m)
	, m_width(2.5f)				//ボートの幅(m)
	, m_height(1.5f)			//ボートの高さ(m)
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
	UpdateWaterInteraction(dt);

	//姿勢制御
	UpdateRotation(dt);
	UpdateRoll(dt);
	UpdatePitch(dt);
	UpdateBobbing(dt);

}

void Boat::UpdatePhysics(float deltaTime) {
	//推進力の計算
	float enginePowerRatio = m_engine.GetCurrentPower() / m_engine.GetMaxPower(); //単純化のため、速度に比例する力とする

	float baseForce = 8000.0f; //最低限の推進力
	float engineForce = baseForce + enginePowerRatio; //エンジン出力に応じた推進力


	//リバースギア時は力を反転
	if (m_isReverse) {
		engineForce *= -0.6f; //リバースは前進の60%の力
	}

	//前進方向の力quaternion版
	Vector3 forwardDir = GetForwardQ();
	Vector3 thrustForce = forwardDir * engineForce;

	//ブレーキ力
	Vector3 brakeForce = -m_velocity * (m_brakeInput * 5.0f); //ブレーキ力は速度に比例

	//水の抵抗力
	float speedSquared = m_velocity.LengthSquared();
	Vector3 dragForce = -m_velocity * (m_waterDrag * speedSquared * 0.001f); //速度の2乗に比例

	//合計力
	Vector3 totalForce = thrustForce + brakeForce + dragForce;

	//加速度 = 力 / 質量
	m_acceleration = totalForce / m_mass;

	//速度更新
	m_velocity += m_acceleration * deltaTime;

	//速度制限
	Vector3 forwardVelocity = forwardDir * m_velocity.Dot(forwardDir); //前進成分
	float forwardSpeed = forwardVelocity.Length();
	bool movingForward = m_velocity.Dot(forwardDir) > 0.0f;

	if (movingForward && forwardSpeed > m_maxSpeed) {
		//前進速度制限
		Vector3 lateralVelocity = m_velocity - forwardVelocity; //横成分
		m_velocity = forwardDir * m_maxSpeed + lateralVelocity;
	} else if (!movingForward && forwardSpeed > m_maxReverseSpeed) {
		//後退速度制限
		Vector3 lateralVelocity = m_velocity - forwardVelocity; //横成分
		m_velocity = forwardDir * -m_maxReverseSpeed + lateralVelocity;
	}

	//位置更新
	m_position += m_velocity * deltaTime;
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
		float currentSpeed = m_velocity.Length();
		float speedFactor = std::min(currentSpeed / 10.0f, 1.0f); //速度に応じて旋回率を減少

		//前進・後退の判定
		Vector3 forwardDir = GetForwardQ();
		bool movingForward = m_velocity.Dot(forwardDir) > 0.0f;

		//後退時は旋回方向を逆にする
		float steeringMultiplier = movingForward ? 1.0f : -1.0f;

		//旋回速度(速度に応じて調整)
		float turnRate = m_steeringInput * steeringMultiplier * m_maxTurnRate * speedFactor * deltaTime;

		//y軸回転のクォータニオンを作成
		Vector4 yawQuat = Vector4::FromAxisAngle(Vector3::UP, turnRate);

		//現在のクォータニオンに回転を適用
		m_quaternion = yawQuat * m_quaternion;
		m_quaternion.Normalize();

		//角速度の更新
		m_angularVelocity.y = turnRate / deltaTime;

		//ロール角の目標値設定
		m_targetRoll = -m_steeringInput * MAX_ROLL_ANGLE * speedFactor;
	} else {
		//ステアリングがない場合、角速度を減衰
		m_angularVelocity.y *= 0.95f;
		//ロール角の目標値をゼロに戻す
		m_targetRoll = 0.0f;
	}
}

void Boat::ApplyForces(float deltaTime) {
	//この関数は将来的に外部からの力を適用する際に使用予定
	//現在は物理計算が完結しているため未使用
}

void Boat::UpdateRoll(float deltaTime) {
	//現在のロール角を取得
	Vector3 currentEuler = m_quaternion.ToEuler();

	//ロール角を目標値に近づける
	float rolldifference = m_targetRoll - currentEuler.z;
	float rollChangeRate = 3.0f; //ロール変化速度

	float newRoll = currentEuler.z + rolldifference * rollChangeRate * deltaTime;
	newRoll = std::clamp(newRoll, -MAX_ROLL_ANGLE, MAX_ROLL_ANGLE);

	//新しいロール角でクォータニオンを更新
	m_quaternion = Vector4::FromEuler(currentEuler.y, currentEuler.x, newRoll);
}

void Boat::UpdatePitch(float deltaTime) {
	//加速度に基づいてピッチ角を調整
	Vector3 forwardDir = GetForwardQ();
	float forwardAccel = m_acceleration.Dot(forwardDir);

	//目標ピッチ角を計算(加速時に上向き、減速時に下向き)
	m_targetPitch = -forwardAccel * 0.02f; //調整係数
	m_targetPitch = std::clamp(m_targetPitch, -MAX_PITCH_ANGLE, MAX_PITCH_ANGLE);

	//現在のピッチ角を取得
	Vector3 currentEuler = m_quaternion.ToEuler();

	//ピッチ角を目標値に近づける
	float pitchDifference = m_targetPitch - currentEuler.x;
	float pitchChangeRate = 2.0f; //ピッチ変化速度

	float newPitch = currentEuler.x + pitchDifference * pitchChangeRate * deltaTime;
	newPitch = std::clamp(newPitch, -MAX_PITCH_ANGLE, MAX_PITCH_ANGLE);

	//新しいピッチ角でクォータニオンを更新
	m_quaternion = Vector4::FromEuler(currentEuler.y, newPitch, currentEuler.z);
}

void Boat::UpdateBobbing(float deltaTime) {
	if (!m_water) return;

	//波による上下動の計算
	m_bobPhase += deltaTime * 2.0f; //ボビングの速度

	//現在の水面の高さを取得
	float waterHeight = m_water->GetWaterHeight(m_position);

	//ボートの底部が水面に接触するように調整
	float targetY = waterHeight + m_height * 0.3f; //ボートの高さの30%を水中に保つ

	//y位置を滑らかに調整
	float yDifference = targetY - m_position.y;
	m_position.y += yDifference * 5.0f * deltaTime; //調整速度
}
