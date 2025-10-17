#pragma once

#include "racingBoat.h"

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

	TargetPoint m_currentTarget = TargetPoint::EAST_BUOY_SOUTH;
	Vector3 m_targetPosition = Vector3(150.0f, 0.0f, -20.0f);

	//目標ブイ位置
	Vector3 m_easeBuoyPosition = Vector3(150.0f, 0.0f, 0.0f);
	Vector3 m_westBuoyPosition = Vector3(-150.0f, 0.0f, 0.0f);

	//制御パラメータ
	float m_targetThrottle = 0.0f;
	float m_targetSteering = 0.0f;
	float m_targetBrake = 0.0f;

	//平滑化パラメータ
	float m_currentThrottle = 0.0f;
	float m_currentSteering = 0.0f;
	float m_currentBrake = 0.0f;
	float m_throttleSmoothRate = 3.0f;
	float m_steeringSmoothRate = 4.0f;

	//AI制御パラメータ
	float m_targetSpeed = 30.0f; //目標速度(m/s)
	float m_buoyOuterRadius = 30.0f; //ブイ外側半径
	float m_waypointReachDistance = 35.0f; //ブイ通過距離
	float m_maxSteerAngle = 1.0f; //最大ステアリング角度

	//壁回避パラメータ
	float m_wallAvoidDistance = 20.0f; //壁回避距離
	float m_wallAvoidStrength = 2.0f; //壁回避強度

	//AI制御更新
	void UpdateAI(double deltaTime);
	void UpdateTargetPoint();
	bool HasReachedWaypoint() const;
	Vector3 CalculateWallAvoidance() const;
	void CalculateSteering();
	void CalculateThrottle();
	void SmoothControls(double deltaTime);
	float CalculateAngleToTarget() const;
	float NormalizeAngle(float angle) const;
};
