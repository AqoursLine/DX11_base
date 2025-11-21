#pragma once

#include "boat.h"

class RacingBoat : public Boat {
public:
	RacingBoat() = default;
	~RacingBoat() = default;


	void SetThrottle(float throttle) override;

	//レース状態管理
	void SetStarting(bool isStarting) { m_isStarted = isStarting; }
	bool IsStarted() const { return m_isStarted; }

	//スタートゲート通過管理
	void SetPassedStartGate(bool isPassed) { m_isPassedStartGate = isPassed; }
	bool IsPassedStartGate() const { return m_isPassedStartGate; }

	//ゴールゲート通過管理
	void SetPassedGoalGate(bool isPassed) { m_isPassedGoalGate = isPassed; }
	bool IsPassedGoalGate() const { return m_isPassedGoalGate; }

	//周回数更新準備フラグ管理
	void SetLapUpdateReady(bool isReady) { m_isLapUpdateReady = isReady; }
	bool IsLapUpdateReady() const { return m_isLapUpdateReady; }

	//レーンインデックス管理
	void SetLaneIndex(int index);
	int GetLaneIndex() const { return m_laneIndex; }

	//周回数管理
	void SetLapCount(int count) { m_lapCount = count; }
	int GetLapCount() const { return m_lapCount; }

	//ゴール処理
	void FinishRace();

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;	

	Vector2 GetSceneBoundsMin() const override;
	Vector2 GetSceneBoundsMax() const override;

	Vector4 m_boatColor = Vector4::ONE; //ボートカラー

private:
	class RaceManager* m_raceManager = nullptr;

	class ModelRenderer* m_model = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	bool m_isStarted = false;
	bool m_isPassedStartGate = false; //スタートゲート通過フラグ
	bool m_isPassedGoalGate = false;  //ゴールゲート通過フラグ
	bool m_isLapUpdateReady = true; //周回数更新準備フラグ

	int m_laneIndex = 0; //レーンインデックス
	int m_lapCount = 0; //周回数

	float m_finishTime = 0.0f; //ゴールタイム
};
