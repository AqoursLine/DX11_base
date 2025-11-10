#pragma once

#include "racingBoat.h"
#include <random>

class CPUBoat : public RacingBoat {
public:
	CPUBoat() = default;
	~CPUBoat() = default;

protected:
	bool Initialize() override;
	void Update(double deltaTime) override;

private:
	//AI制御用
	enum class TargetPoint {
		EAST_BUOY_SOUTH,
		EAST_BUOY_NORTH,
		WEST_BUOY_NORTH,
		WEST_BUOY_SOUTH,
		GOAL_GATE
	};

	//コース区間判定用
	enum class CourseSection {
		SECTION_1,	//東ブイ南→北(東側南エリア)
		SECTION_2,	//東ブイ北→西ブイ北(北エリア)
		SECTION_3,	//西ブイ北→南(西側北エリア)
		SECTION_4,	//西ブイ南→ゴールゲート(南エリア)
		SECTION_5	//ゴールゲート→東ブイ南(南エリア)
	};

	TargetPoint m_currentTarget = TargetPoint::EAST_BUOY_SOUTH;
	Vector3 m_targetPosition = Vector3(150.0f, 0.0f, -20.0f);

	//ブイの実際の座標
	Vector3 m_eastBuoyPos = Vector3(150.0f, 0.0f, 0.0f);
	Vector3 m_westBuoyPos = Vector3(-150.0f, 0.0f, 0.0f);

	//制御パラメータ
	float m_targetThrottle = 0.0f;
	float m_targetSteering = 0.0f;
	float m_targetBrake = 0.0f;

	//平滑化パラメータ
	float m_currentThrottle = 0.0f;
	float m_currentSteering = 0.0f;
	float m_currentBrake = 0.0f;
	float m_throttleSmoothRate = 3.0f;	//スロットル平滑化速度
	float m_steeringSmoothRate = 4.0f;	//ステアリング平滑化速度

	//AI制御パラメータ
	float m_targetSpeed = 30.0f;		//目標速度
	float m_buoyOuterRadius = 35.0f;	//ブイの外側通過半径
	float m_maxSteerAngle = 1.0f;		//最大ステアリング角

	//壁回避パラメータ
	float m_wallAvoidDistance = 30.0f;	//壁回避開始距離
	float m_wallAvoidStrength = 1.0f;	//壁回避強度

	//ボート回避パラメータ
	float m_boatAvoidDistance = 30.0f;	//ボート回避開始距離
	float m_boatAvoidStrength = 1.5f;	//ボート回避強度

	//ランダム挙動パラメータ
	std::mt19937 m_randomEngine;				//乱数エンジン
	float m_speedVariation = 0.0f;				//速度変動量
	float m_targetPositionOffsetX = 0.0f;		//目標位置Xオフセット
	float m_targetPositionOffsetZ = 0.0f;		//目標位置Zオフセット
	float m_steeringResponseVariation = 0.0f;	//ステアリング応答変動
	float m_throttleResponseVariation = 0.0f;	//スロットル応答変動
	float m_mistakeTimer = 0.0f;				//ミス行動タイマー
	float m_mistakeDuration = 0.0f;				//ミス行動継続時間
	float m_mistakeSteeringOffset = 0.0f;		//ミス行動ステアリングオフセット

	//AI制御更新
	void UpdateAI(double deltaTime);
	void UpdateMistakeBehavior(double deltaTime);
	void UpdateTargetPoint();
	void CalculateSteering();
	void CalculateThrottle();
	void SmoothControls(double deltaTime);
	float CalculateAngleToTarget() const;
	float NormalizeAngle(float angle) const;

	//区間判定
	CourseSection GetCurrentCourseSection() const;
	bool ShouldSwitchToNextTarget() const;

	//回避機能
	Vector3 CalculateWallAvoidance() const;
	Vector3 CalculateBoatAvoidance() const;

	//ランダム挙動
	void InitializeRandomBehavior();						//ランダム挙動初期化
	Vector3 ApplyRandomOffsets(const Vector3& position) ;	//目標位置にランダムオフセット適用
	float GetRandomFloat(float min, float max);				//乱数取得

};
