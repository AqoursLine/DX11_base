#include "cpuBoat.h"

#include <algorithm>

bool CPUBoat::Initialize() {
	if (!RacingBoat::Initialize()) {
		return false;
	}

	//初期目標設定
	m_currentTarget = TargetPoint::EAST_BUOY_SOUTH;
	m_targetPosition = Vector3(m_easeBuoyPosition.x, 0.0f, -m_buoyOuterRadius);

	return true;
}

void CPUBoat::Update(double deltaTime) {
	//AI制御更新
	UpdateAI(deltaTime);

	//入力を車両制御に反映
	SetThrottle(m_currentThrottle);
	SetSteering(m_currentSteering);
	SetBrake(m_currentBrake);

	RacingBoat::Update(deltaTime);
}

void CPUBoat::UpdateAI(double deltaTime) {
	//レース開始前は制御しない
	if (!IsStarted()) {
		m_targetThrottle = 0.0f;
		m_targetSteering = 0.0f;
		m_targetBrake = 0.0f;
		return;
	}

	//ゴール後は制御しない
	if (IsPassedGoalGate()) {
		m_targetThrottle = 0.0f;
		m_targetSteering = 0.0f;
		m_targetBrake = 1.0f;
		SmoothControls(deltaTime);
		return;
	}

	//目標ブイ更新
	UpdateTargetPoint();

	//ステアリング計算
	CalculateSteering();

	//スロットル計算
	CalculateThrottle();

	//制御入力平滑化
	SmoothControls(deltaTime);
}

void CPUBoat::UpdateTargetPoint() {
	//現在の通過点に到達したかチェック
	if (!HasReachedWaypoint()) {
		return;
	}

	//次の通過点に切り替え
	switch (m_currentTarget) {
		case CPUBoat::TargetPoint::EAST_BUOY_SOUTH:
			//次は東ブイ北側
			m_currentTarget = CPUBoat::TargetPoint::EAST_BUOY_NORTH;
			m_targetPosition = Vector3(m_easeBuoyPosition.x, 0.0f, m_buoyOuterRadius);
			break;

		case CPUBoat::TargetPoint::EAST_BUOY_NORTH:
			//次は西ブイ北側
			m_currentTarget = CPUBoat::TargetPoint::WEST_BUOY_NORTH;
			m_targetPosition = Vector3(m_westBuoyPosition.x, 0.0f, m_buoyOuterRadius);
			break;

		case CPUBoat::TargetPoint::WEST_BUOY_NORTH:
			//次は西ブイ南側
			m_currentTarget = CPUBoat::TargetPoint::WEST_BUOY_SOUTH;
			m_targetPosition = Vector3(m_westBuoyPosition.x, 0.0f, -m_buoyOuterRadius);
			break;

		case CPUBoat::TargetPoint::WEST_BUOY_SOUTH:
			//次はゴールゲート
			m_currentTarget = CPUBoat::TargetPoint::GOAL_GATE;
			m_targetPosition = Vector3(0.0f, 0.0f, -30.0f);
			break;

		case CPUBoat::TargetPoint::GOAL_GATE:
			//次は東ブイ南側
			m_currentTarget = CPUBoat::TargetPoint::EAST_BUOY_SOUTH;
			m_targetPosition = Vector3(m_easeBuoyPosition.x, 0.0f, -m_buoyOuterRadius);
			break;
	}
}

bool CPUBoat::HasReachedWaypoint() const {
	//現在の通過点までの距離
	Vector4 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f; //水平距離のみ考慮
	float distance = toTarget.Length();

	//到達判定距離以内か
	return distance < m_waypointReachDistance;
}

Vector3 CPUBoat::CalculateWallAvoidance() const {
	//シーン境界を取得
	Vector2 sceneMin = GetSceneBoundsMin();
	Vector2 sceneMax = GetSceneBoundsMax();

	Vector3 avoidance(0.0f, 0.0f, 0.0f);

	//各壁までの距離をチェック
	float distToEastWall = sceneMax.x - m_position.x;
	float distToWestWall = m_position.x - sceneMin.x;
	float distToNorthWall = sceneMax.y - m_position.z;
	float distToSouthWall = m_position.z - sceneMin.y;

	//東壁
	if (distToEastWall < m_wallAvoidDistance) {
		float avoidForce = (m_wallAvoidDistance - distToEastWall) / m_wallAvoidDistance;
		avoidance.x -= avoidForce * m_wallAvoidStrength;
	}

	//西壁
	if (distToWestWall < m_wallAvoidDistance) {
		float avoidForce = (m_wallAvoidDistance - distToWestWall) / m_wallAvoidDistance;
		avoidance.x += avoidForce * m_wallAvoidStrength;
	}

	//北壁
	if (distToNorthWall < m_wallAvoidDistance) {
		float avoidForce = (m_wallAvoidDistance - distToNorthWall) / m_wallAvoidDistance;
		avoidance.z -= avoidForce * m_wallAvoidStrength;
	}

	//南壁
	if (distToSouthWall < m_wallAvoidDistance) {
		float avoidForce = (m_wallAvoidDistance - distToSouthWall) / m_wallAvoidDistance;
		avoidance.z += avoidForce * m_wallAvoidStrength;
	}

	return avoidance;
}

void CPUBoat::CalculateSteering() {
	//壁回避ベクトルを計算
	Vector3 wallAvoidance = CalculateWallAvoidance();

	//目標への方向ベクトル
	Vector3 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f;
	toTarget.Normalize();

	//壁回避が必要な場合は目標方向と合成
	Vector3 desiredDirection = toTarget;
	if (wallAvoidance.Length() > 0.01f) {
		//壁回避ベクトルを正規化
		wallAvoidance.Normalize();

		//目標方向と壁回避方向をブレンド
		//壁が近い程壁回避成分を強くする
		float wallAvoidWeight = std::min(wallAvoidance.Length() * 0.2f, 0.8f);
		desiredDirection = toTarget * (1.0f - wallAvoidWeight) + wallAvoidance * wallAvoidWeight;
		desiredDirection.Normalize();
	}

	//ボートの前方ベクトル
	Vector3 forward = GetYawRotation().RotateVector(Vector3::FORWARD);

	//内積から角度を計算
	float dot = forward.Dot(desiredDirection);
	float angleToTarget = std::acos(std::clamp(dot, -1.0f, 1.0f));

	//外積から符号を決定
	Vector3 cross = forward.Cross(desiredDirection);
	if (cross.y < 0.0f) {
		angleToTarget = -angleToTarget;
	}

	angleToTarget = NormalizeAngle(angleToTarget);

	//ステアリング目標値を計算
	m_targetSteering = std::clamp(angleToTarget / m_maxSteerAngle, -1.0f, 1.0f);

	//カーブの鋭さに応じてステアリングを調整
	float absAngle = std::abs(angleToTarget);

	if (absAngle > XM_PIDIV4) {
		//45度以上の鋭いカーブ
		m_targetSteering *= 1.3f;
		m_targetSteering = std::clamp(m_targetSteering, -1.0f, 1.0f);
	} else if (absAngle > XM_PI / 6.0f) {
		//30度以上の中程度のカーブ
		m_targetSteering *= 1.15f;
		m_targetSteering = std::clamp(m_targetSteering, -1.0f, 1.0f);
	}
}

void CPUBoat::CalculateThrottle() {
	//現在の速度取得
	float currentSpeed = GetSpeed();

	//目標への角度
	float angleToTarget = CalculateAngleToTarget();
	float absAngle = std::abs(angleToTarget);

	//壁回避が必要かチェック
	Vector3 wallAvoidance = CalculateWallAvoidance();
	bool isAvoidingWall = wallAvoidance.Length() > 0.5f;

	//基本スロットル(目標速度との差に基づく)
	float speedDiff = m_targetSpeed - currentSpeed;
	m_targetThrottle = std::clamp(speedDiff * 0.1f, 0.0f, 1.0f);

	//壁回避中はスロットルを抑制
	if (isAvoidingWall) {
		m_targetThrottle *= 0.6f;
		if (currentSpeed > m_targetSpeed * 0.5f) {
			m_targetBrake = 0.3f;
		} else {
			m_targetBrake = 0.0f;
		}
		return;
	}

	//カーブの鋭さに応じてスロットルを調整
	if (absAngle > XM_PI / 3.0f) {
		//60度以上の鋭いカーブ
		m_targetThrottle *= 0.5f;
		//ブレーキをかける
		if (currentSpeed > m_targetSpeed * 0.6f) {
			m_targetBrake = 0.4f;
		} else {
			m_targetBrake = 0.0f;
		}
	} else if (absAngle > XM_PIDIV4) {
		//45度以上の中程度のカーブ
		m_targetThrottle *= 0.7f;
		if (currentSpeed > m_targetSpeed * 0.75f) {
			m_targetBrake = 0.2f;
		} else {
			m_targetBrake = 0.0f;
		}
	} else {
		m_targetBrake = 0.0f;
	}

	//目標が遠く、方向があってい場合はフルスロットル
	Vector4 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f;
	float distanceToTarget = toTarget.Length();

	if (distanceToTarget > 50.f && absAngle < XM_PI / 6.0f) {
		//距離50m以上、30度以内
		m_targetThrottle = 1.0f;
	}

	//最低スロットル確保
	if (m_targetThrottle < 0.3f && m_targetBrake < 0.1f) {
		m_targetThrottle = 0.3f;
	}
}

void CPUBoat::SmoothControls(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//ステアリング平滑化
	float steerRate = std::min(m_steeringSmoothRate * dt, 1.0f);

	//ステアリングを戻す時は速く
	if (std::abs(m_targetSteering) < std::abs(m_currentSteering)) {
		steerRate *= 1.5f;
		steerRate = std::min(steerRate, 1.0f);
	}

	m_currentSteering = std::lerp(m_currentSteering, m_targetSteering, steerRate);

	//スロットル平滑化
	float throttleRate = std::min(m_throttleSmoothRate * dt, 1.0f);
	m_currentThrottle = std::lerp(m_currentThrottle, m_targetThrottle, throttleRate);

	//ブレーキはそのまま
	m_currentBrake = m_targetBrake;
}

float CPUBoat::CalculateAngleToTarget() const {
	//目標への方向ベクトル
	Vector3 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f; //水平成分のみ

	if (toTarget.Length() < 0.01f) {
		return 0.0f; //目標にほぼ到達
	}

	toTarget.Normalize();

	//ボートの前方ベクトル
	Vector3 forward = GetYawRotation().RotateVector(Vector3::FORWARD);

	//内積から角度を計算
	float dot = forward.Dot(toTarget);
	float angle = std::acos(std::clamp(dot, -1.0f, 1.0f));

	//外積から符号を決定
	Vector3 cross = forward.Cross(toTarget);
	if (cross.y < 0.0f) {
		angle = -angle;
	}

	return NormalizeAngle(angle);
}

float CPUBoat::NormalizeAngle(float angle) const {
	while (angle > XM_PI) {
		angle -= XM_2PI;
	}
	while (angle < -XM_PI) {
		angle += XM_2PI;
	}
	return angle;
}
