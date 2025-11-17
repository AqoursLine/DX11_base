#include "cpuBoat.h"

#include "system.h"
#include "manager.h"
#include "scene.h"

#include <algorithm>
#include <chrono>

constexpr float M_PI = 3.14159265358979323846f;
constexpr float M_PIDIV2 = 1.57079632679489661923f;
constexpr float M_PIDIV3 = 1.04719755119659774615f;
constexpr float M_PIDIV4 = 0.78539816339744830961f;
constexpr float M_PIDIV6 = 0.52359877559829887308f;

/// <summary>
/// CPUボート初期化
/// </summary>
/// <returns>初期化完了</returns>
bool CPUBoat::Initialize() {
	if (!RacingBoat::Initialize()) {
		return false;
	}

	//ランダム挙動の初期化
	InitializeRandomBehavior();

	//初期目標設定：東ブイの南側
	m_currentTarget = TargetPoint::EAST_BUOY_SOUTH;
	m_targetPosition = ApplyRandomOffsets(Vector3(m_eastBuoyPos.x, 0.0f, -m_buoyOuterRadius));

	return true;
}

/// <summary>
/// CPUボートの更新処理
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
void CPUBoat::Update(double deltaTime) {
	//AI制御更新
	UpdateAI(deltaTime);

	//入力を車両制御に反映
	SetThrottle(m_currentThrottle);
	SetSteering(m_currentSteering);
	SetBrake(m_currentBrake);

	RacingBoat::Update(deltaTime);
}

/// <summary>
/// AI制御更新
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
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

	//ミス挙動の更新
	UpdateMistakeBehavior(deltaTime);

	//目標通過点の更新
	UpdateTargetPoint();

	//ステアリング計算
	CalculateSteering();

	//スロットル計算
	CalculateThrottle();

	//制御入力の平滑化
	SmoothControls(deltaTime);

}

/// <summary>
/// ミス挙動の更新
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
void CPUBoat::UpdateMistakeBehavior(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//ミス中の場合
	if (m_mistakeTimer > 0.0f) {
		m_mistakeTimer -= dt;
		if (m_mistakeTimer <= 0.0f) {
			//ミス終了
			m_mistakeSteeringOffset = 0.0f;
		}
		return;
	}

	//ランダムにミスを発生させる
	float mistakeProbability = dt / 10.0f; // 平均10秒に1回ミス
	if (GetRandomFloat(0.0f, 1.0f) < mistakeProbability) {
		//ミス挙動開始
		m_mistakeDuration = GetRandomFloat(0.3f, 1.0f); // ミス継続時間
		m_mistakeTimer = m_mistakeDuration;
		m_mistakeSteeringOffset = GetRandomFloat(-0.4f, 0.4f); // ミスステアリングオフセット
	}
}

/// <summary>
/// 目標通過点の更新
/// </summary>
void CPUBoat::UpdateTargetPoint() {
	//現在の区間に基づいて次のターゲットに切り替えるべきか判定
	if (!ShouldSwitchToNextTarget()) {
		return;
	}

	//次の通過点に切り替える
	switch (m_currentTarget) {
		case CPUBoat::TargetPoint::EAST_BUOY_SOUTH:
			//次は東ブイ北側
			m_currentTarget = TargetPoint::EAST_BUOY_NORTH;
			m_targetPosition = ApplyRandomOffsets(Vector3(m_eastBuoyPos.x, 0.0f, m_buoyOuterRadius));
			break;
		case CPUBoat::TargetPoint::EAST_BUOY_NORTH:
			//次は西ブイ北側
			m_currentTarget = TargetPoint::WEST_BUOY_NORTH;
			m_targetPosition = ApplyRandomOffsets(Vector3(m_westBuoyPos.x, 0.0f, m_buoyOuterRadius));
			break;
		case CPUBoat::TargetPoint::WEST_BUOY_NORTH:
			//次は西ブイ南側
			m_currentTarget = TargetPoint::WEST_BUOY_SOUTH;
			m_targetPosition = ApplyRandomOffsets(Vector3(m_westBuoyPos.x, 0.0f, -m_buoyOuterRadius));
			break;
		case CPUBoat::TargetPoint::WEST_BUOY_SOUTH:
			//次は東ブイ南側
			m_currentTarget = TargetPoint::EAST_BUOY_SOUTH;
			m_targetPosition = ApplyRandomOffsets(Vector3(m_eastBuoyPos.x, 0.0f, -m_buoyOuterRadius));
			break;
	}
}

/// <summary>
/// ステアリングを計算
/// </summary>
void CPUBoat::CalculateSteering() {
	//壁回避ベクトルを計算
	Vector3 wallAvoid = CalculateWallAvoidance();
	
	//ボート回避ベクトルを計算
	Vector3 boatAvoid = CalculateBoatAvoidance();

	//目標への方向ベクトル
	Vector3 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f; //水平面のみ
	if (toTarget.Length() < 0.1f) {
		toTarget = Vector3::FORWARD;
	}
	toTarget.Normalize();

	//壁回避の強さを保存
	float wallAvoidStrength = wallAvoid.Length();
	//ボート回避の強さを保存
	float boatAvoidStrength = boatAvoid.Length();

	//合成ベクトルを計算
	Vector3 desiredDirection = toTarget;

	//壁回避を加算
	if (wallAvoidStrength > 0.01f) {
		wallAvoid.y = 0.0f;
		wallAvoid.Normalize();

		//壁が近いほど壁回避を優先（最大50%）
		float wallAvoidWeight = std::min(wallAvoidStrength * 0.2f, 0.5f);
		desiredDirection = desiredDirection * (1.0f - wallAvoidWeight) + wallAvoid * wallAvoidWeight;
	}

	//ボート回避を加算
	if (boatAvoidStrength > 0.01f) {
		boatAvoid.y = 0.0f;
		boatAvoid.Normalize();
		//ボートが近いほどボート回避を優先（最大40%）
		float boatAvoidWeight = std::min(boatAvoidStrength * 0.3f, 0.4f);
		desiredDirection = desiredDirection * (1.0f - boatAvoidWeight) + boatAvoid * boatAvoidWeight;
	}

	desiredDirection.Normalize();

	//ボートの前方向ベクトル
	Vector3 forward = GetYawRotation().RotateVector(Vector3::FORWARD);

	//内積から角度を計算
	float dot = forward.Dot(desiredDirection);
	float angleToDesired = std::acos(std::clamp(dot, -1.0f, 1.0f));

	//外積で左右を判定
	Vector3 cross = forward.Cross(desiredDirection);
	if (cross.y < 0.0f) {
		angleToDesired = -angleToDesired;
	}

	angleToDesired = NormalizeAngle(angleToDesired);

	//ステアリング目標値を計算
	m_targetSteering = std::clamp(angleToDesired / m_maxSteerAngle, -1.0f, 1.0f);

	//ミス挙動を適用
	m_targetSteering += m_mistakeSteeringOffset;
	m_targetSteering = std::clamp(m_targetSteering, -1.0f, 1.0f);

	//カーブの鋭さに応じてステアリングを調整
	float absAngle = std::abs(angleToDesired);

	if (absAngle > M_PIDIV4) {
		//45度以上：急カーブ
		m_targetSteering *= 1.3f;
		m_targetSteering = std::clamp(m_targetSteering, -1.0f, 1.0f);
	} else if (absAngle > M_PIDIV6) {
		//30度以上：中カーブ
		m_targetSteering *= 1.15f;
		m_targetSteering = std::clamp(m_targetSteering, -1.0f, 1.0f);
	}
}

/// <summary>
/// スロットルを計算
/// </summary>
void CPUBoat::CalculateThrottle() {
	//現在の速度取得
	float currentSpeed = GetSpeed();

	//目標の角度
	float angleToTarget = CalculateAngleToTarget();
	float absAngle = std::abs(angleToTarget);

	//壁回避が必要かチェック
	Vector3 wallAvoid = CalculateWallAvoidance();
	bool isWallAvoidingWall = wallAvoid.Length() > 0.5f;

	//ボート回避が必要かチェック
	Vector3 boatAvoid = CalculateBoatAvoidance();
	bool isAvoidingBoat = boatAvoid.Length() > 0.5f;

	//ランダムな目標速度を適用
	float adjustedTargetSpeed = m_targetSpeed + m_speedVariation;
	adjustedTargetSpeed = std::clamp(adjustedTargetSpeed, 20.0f, 35.0f);

	//基本スロットル（目標速度との差に基づく）
	float speedDiff = adjustedTargetSpeed - currentSpeed;
	m_targetThrottle = std::clamp(speedDiff * 0.1f, 0.0f, 1.0f);

	//壁回避中は減速
	if (isWallAvoidingWall) {
		m_targetThrottle *= 0.5f;
		if (currentSpeed > adjustedTargetSpeed * 0.5f) {
			m_targetBrake = 0.3f;
		} else {
			m_targetBrake = 0.0f;
		}
		return;
	}

	//ボート回避中は減速
	if (isAvoidingBoat) {
		m_targetThrottle *= 0.7f;
		if (currentSpeed > adjustedTargetSpeed * 0.7f) {
			m_targetBrake = 0.2f;
		} else {
			m_targetBrake = 0.0f;
		}
		return;
	}

	//カーブの鋭さに応じて速度調整
	if (absAngle > M_PIDIV3) {
		//60度以上：急カーブ
		m_targetThrottle *= 0.3f;
		//速度が速すぎる場合はブレーキ
		if (currentSpeed > adjustedTargetSpeed * 0.5f) {
			m_targetBrake = 0.6f;
		} else {
			m_targetBrake = 0.0f;
		}
	} else if (absAngle > M_PIDIV4) {
		//45度以上：中カーブ
		m_targetThrottle *= 0.5f;
		if (currentSpeed > adjustedTargetSpeed * 0.65f) {
			m_targetBrake = 0.4f;
		} else {
			m_targetBrake = 0.0f;
		}
	} else if (absAngle > M_PIDIV6) {
		//30度以上：緩カーブ
		m_targetThrottle *= 0.7f;
		if (currentSpeed > adjustedTargetSpeed * 0.8f) {
			m_targetBrake = 0.2f;
		} else {
			m_targetBrake = 0.0f;
		}
	} else {
		m_targetBrake = 0.0f;
	}

	//目標が遠く、方向が合っている場合はフルスロットル
	Vector3 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f;
	float distanceToTarget = toTarget.Length();

	if (distanceToTarget > 50.0f && absAngle < M_PIDIV6) {
		//30度以内
		m_targetThrottle = 1.0f;
	}

	//最低スロットルを確保
	if (m_targetThrottle < 0.3f && m_targetBrake < 0.1f) {
		m_targetThrottle = 0.3f;
	}
}

/// <summary>
/// スムーズな操作を適用
/// </summary>
/// <param name="deltaTime"></param>
void CPUBoat::SmoothControls(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//ステアリング平滑化
	float steerRate = std::min(m_steeringSmoothRate * dt, 1.0f);

	//ステアリングを戻す時は速くする
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

/// <summary>
/// 目標への角度を計算
/// </summary>
/// <returns>目標への角度</returns>
float CPUBoat::CalculateAngleToTarget() const {
	//目標への方向ベクトル
	Vector3 toTarget = m_targetPosition - m_position;
	toTarget.y = 0.0f; //水平面のみ

	if (toTarget.Length() < 0.1f) {
		return 0.0f;
	}

	toTarget.Normalize();

	//ボートの前方向ベクトル
	Vector3 forward = GetYawRotation().RotateVector(Vector3::FORWARD);

	//内積から角度を計算
	float dot = forward.Dot(toTarget);
	float angle = std::acos(std::clamp(dot, -1.0f, 1.0f));

	//外積で左右を判定
	Vector3 cross = forward.Cross(toTarget);
	if (cross.y < 0.0f) {
		angle = -angle;
	}

	return NormalizeAngle(angle);
}


/// <summary>
/// 角度を正規化
/// </summary>
/// <param name="angle">正規化前の角度</param>
/// <returns>正規化後の角度</returns>
float CPUBoat::NormalizeAngle(float angle) const {
	//-π ～ π の範囲に正規化
	while (angle > XM_PI) angle -= XM_2PI;
	while (angle < -XM_PI) angle += XM_2PI;
	return angle;
}

/// <summary>
/// 現在のコース区間を取得
/// </summary>
/// <returns>現在のコース区間</returns>
CPUBoat::CourseSection CPUBoat::GetCurrentCourseSection() const {
	//現在位置から区間を判定
	float x = m_position.x;
	float z = m_position.z;

	//南エリア
	if (z < 0.0f && x > m_westBuoyPos.x && x < m_eastBuoyPos.x - 40.0f) {
		return CourseSection::SECTION_4;
	}
	//北エリア
	else if (z > 0.0f && x > m_westBuoyPos.x + 40.0f && x < m_eastBuoyPos.x) {
		return CourseSection::SECTION_2;
	}
	//東エリア
	else if (x > 0.0f) {
		return CourseSection::SECTION_1;
	}
	//西エリア
	else {
		return CourseSection::SECTION_3;
	}
}

/// <summary>
/// 次のターゲットに切り替えるべきか判定
/// </summary>
/// <returns>切り替えるべき</returns>
bool CPUBoat::ShouldSwitchToNextTarget() const {
	CourseSection currentSection = GetCurrentCourseSection();

	//現在の目標と区間の対応を確認
	switch (m_currentTarget) {
		case CPUBoat::TargetPoint::EAST_BUOY_SOUTH:
			//区間1（東側）に入ったら次へ
			return currentSection == CourseSection::SECTION_1;
		case CPUBoat::TargetPoint::EAST_BUOY_NORTH:
			//区間２（北側）に入ったら次へ
			return currentSection == CourseSection::SECTION_2;
		case CPUBoat::TargetPoint::WEST_BUOY_NORTH:
			//区間３（西側北エリア）に入ったら次へ
			return currentSection == CourseSection::SECTION_3;
		case CPUBoat::TargetPoint::WEST_BUOY_SOUTH:
			//区間４（南側）に入ったら次へ
			return currentSection == CourseSection::SECTION_4;
	}

	return false;
}

/// <summary>
/// 壁回避ベクトルを計算
/// </summary>
/// <returns>壁回避ベクトル</returns>
Vector3 CPUBoat::CalculateWallAvoidance() const {
	//シーン境界を取得
	Vector2 boundsMin = GetSceneBoundsMin();
	Vector2 boundsMax = GetSceneBoundsMax();

	Vector3 avoidance(0.0f, 0.0f, 0.0f);

	//各壁までの距離をチェック
	float distToEastWall = boundsMax.x - m_position.x;	//東の壁(x+)
	float distToWestWall = m_position.x - boundsMin.x;	//西の壁(x-)
	float distToNorthWall = boundsMax.y - m_position.z;	//北の壁(z+)
	float distToSouthWall = m_position.z - boundsMin.y;	//南の壁(z-)

	//中央線からの距離も考慮
	float distToCenterLine = std::abs(m_position.z); //中央線(z=0)からの距離
	//中央線が近い場合、遠ざかるように回避力を追加
	if (distToCenterLine < m_wallAvoidDistance) {
		float ratio = distToCenterLine / m_wallAvoidDistance;
		float avoidForce = (1.0f - ratio) * (1.0f - ratio);
		if (m_position.z > 0.0f) {
			//北側にいる場合、南方向に回避
			avoidance.z -= avoidForce * m_wallAvoidStrength * 0.5f;
		} else {
			//南側にいる場合、北方向に回避
			avoidance.z += avoidForce * m_wallAvoidStrength * 0.5f;
		}
	}

	//東の壁が近い場合、西方向に回避（2乗で滑らかに）
	if (distToEastWall < m_wallAvoidDistance) {
		float ratio = distToWestWall / m_wallAvoidDistance;
		float avoidForce = (1.0f - ratio) * (1.0f - ratio);
		avoidance.x -= avoidForce * m_wallAvoidStrength;
	}

	//西の壁が近い場合、東方向に回避（2乗で滑らかに）
	if (distToWestWall < m_wallAvoidDistance) {
		float ratio = distToWestWall / m_wallAvoidDistance;
		float avoidForce = (1.0f - ratio) * (1.0f - ratio);
		avoidance.x += avoidForce * m_wallAvoidStrength;
	}

	//北の壁が近い場合、南方向に回避（2乗で滑らかに）
	if (distToNorthWall < m_wallAvoidDistance) {
		float ratio = distToNorthWall / m_wallAvoidDistance;
		float avoidForce = (1.0f - ratio) * (1.0f - ratio);
		avoidance.z -= avoidForce * m_wallAvoidStrength;
	}

	//南の壁が近い場合、北方向に回避（2乗で滑らかに）
	if (distToSouthWall < m_wallAvoidDistance) {
		float ratio = distToSouthWall / m_wallAvoidDistance;
		float avoidForce = (1.0f - ratio) * (1.0f - ratio);
		avoidance.z += avoidForce * m_wallAvoidStrength;
	}

	return avoidance;
}

/// <summary>
/// ボート回避ベクトルを計算
/// </summary>
/// <returns>ボート回避ベクトル</returns>
Vector3 CPUBoat::CalculateBoatAvoidance() const {
	Vector3 avoidance(0.0f, 0.0f, 0.0f);

	//シーンから他のボートを取得
	auto boats = m_scene->GetGameObjects<RacingBoat>();

	//自分の前方向ベクトル
	Vector3 forward = GetYawRotation().RotateVector(Vector3::FORWARD);

	//各ボートとの距離をチェック
	for (auto* otherBoat : boats) {
		//自分自身はスキップ
		if (otherBoat == this) continue;

		//他のボートまでのベクトル
		Vector3 toOther = otherBoat->GetPosition() - m_position;
		toOther.y = 0.0f; //水平面のみ
		float distance = toOther.Length();

		//回避距離外ならスキップ
		if (distance > m_boatAvoidDistance) continue;

		//前方にいるボートのみ回避
		toOther.Normalize();
		float dotProduct = forward.Dot(toOther);
		if (dotProduct < 0.5f) continue; //約60度以内

		//距離に応じた回避力を計算（近いほど強く回避）
		float ratio = distance / m_boatAvoidDistance;
		float avoidForce = (1.0f - ratio) * (1.0f - ratio);

		//他のボートから離れる方向に回避ベクトルを追加
		Vector3 avoidDir = -toOther;	//反対方向
		avoidance += avoidDir * avoidForce * m_boatAvoidStrength;
	}

	return avoidance;
}

/// <summary>
/// ランダムな挙動を初期化
/// </summary>
void CPUBoat::InitializeRandomBehavior() {
	//乱数エンジンを現在時刻とオブジェクトアドレスでシード
	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count() +
		reinterpret_cast<uintptr_t>(this);

	m_randomEngine.seed(static_cast<unsigned int>(seed));

	//速度のバラツキ
	m_speedVariation = GetRandomFloat(-5.0f, 5.0f);

	//操舵反応速度のバラツキ
	m_steeringResponseVariation = GetRandomFloat(-1.0f, 1.0f);
	m_steeringSmoothRate += m_steeringResponseVariation;
	m_steeringSmoothRate = std::clamp(m_steeringSmoothRate, 2.0f, 7.0f);

	//スロットル反応速度のバラツキ
	m_throttleResponseVariation = GetRandomFloat(-0.5f, 0.5f);
	m_throttleSmoothRate += m_throttleResponseVariation;
	m_throttleSmoothRate = std::clamp(m_throttleSmoothRate, 2.0f, 5.0f);

	//ブイ外側半径のバラツキ
	m_buoyOuterRadius += GetRandomFloat(-5.0f, 5.0f);
	m_buoyOuterRadius = std::clamp(m_buoyOuterRadius, 25.0f, 45.0f);
}

/// <summary>
/// ランダムなオフセットを適用
/// </summary>
/// <param name="position">オフセット前の座標</param>
/// <returns>オフセット後の座標</returns>
Vector3 CPUBoat::ApplyRandomOffsets(const Vector3& position) {
	//各通過点毎異なるランダムオフセットを生成
	float offsetX = GetRandomFloat(20.0f, 30.0f);
	float offsetZ = GetRandomFloat(0, 20.0f);

	//X座標の符号に応じてオフセットを反転
	float signX = position.x / std::abs(position.x);
	offsetX *= -signX;

	offsetZ *= -signX;

	return Vector3(position.x + offsetX, position.y, position.z + offsetZ);
}

float CPUBoat::GetRandomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(m_randomEngine);
}
