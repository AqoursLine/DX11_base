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

	//周回進捗度
	float GetLapProgress() const { return m_lapProgress; }

	//順位設定
	void SetRank(int rank) { m_rank = rank; }
	int GetRank() const { return m_rank; }

	//ゴール処理
	void FinishRace();

	RacingBoat* SetRacerName(const std::string& name) { m_racerName = name; return this; }

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

	float m_lapProgress = 0.0f; //現在の周回進捗度(0.0f ~ 1.0f)

	int m_rank = 0; //順位

	enum ProgressSection {
		FIRST_SOUTH_STRAIGHT = 0,
		EAST_CURVE,
		FIRST_NORTH_STRAIGHT,
		SECOND_NORTH_STRAIGHT,
		WEST_CURVE,
		SECOND_SOUTH_STRAIGHT,
	};
	ProgressSection m_currentSection = FIRST_SOUTH_STRAIGHT;

	float m_finishTime = 0.0f; //ゴールタイム

	std::string m_racerName = "Player"; //レーサー名

	// セクション判定
	void UpdateProgressSection();

	// 周回進捗度計算
	void CalculateLapProgress();

	/// <summary>
	/// セクション進捗度取得(直線)
	/// </summary>
	/// <param name="pos">ゲートポジション</param>
	/// <param name="dir">ゲートの正面方向</param>
	/// <returns>セクション内の進捗度(0.0f ~ 1.0f)</returns>
	float GetSectionProgress(const Vector3& pos, const Vector3& dir);

	/// <summary>
	/// セクション進捗度取得(カーブ)
	/// </summary>
	/// <param name="center">カーブ中心位置</param>
	/// <param name="offset">カーブの開始角度(rad)</param>
	/// <param name="totalAngle">カーブ全体の角度</param>
	/// <returns>カーブ内の進捗度(0.0f ~ 1.0f)</returns>
	float GetSectionProgressCurve(const Vector3& center, float offset, float totalAngle = XM_PI);
};
