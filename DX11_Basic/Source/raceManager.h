#pragma once

#include "gameObject.h"

class RacingBoat;

struct BoatResultData {
	int laneIndex;		// レーンインデックス
	float finishTime;	// ゴールタイム
	Vector4 boatColor;	// ボートカラー
	std::string playerName; // プレイヤー名
};

class RaceManager : public GameObject {
public:
	RaceManager() = default;
	~RaceManager() = default;

	//レースタイム取得
	float GetRaceTime() const { return m_raceTime; }
	//スタート前のカウントダウン時間取得
	float GetStartDelay() const { return m_countDown; }

	//レース開始フラグ取得
	bool IsRaceStarted() const { return m_raceStarted; }
	//レース開始フラグ設定
	void SetRaceStarted(bool started) { m_raceStarted = started; }

	//境界ボックス取得
	Vector2 GetBoundsMin() const { return m_boundsMin; }
	Vector2 GetBoundsMax() const { return m_boundsMax; }

	//完走に必要な周回数取得
	int GetLapCountToFinish() const { return m_lapCountToFinish; }

	RacingBoat* GetRacingBoat(unsigned int laneIndex) const;
	RacingBoat* GetRacingBoatByRank(unsigned int rank) const;

	//レーンごとのタイム記録
	void SetResultData(const BoatResultData& data);
	static const std::vector<BoatResultData>& GetResultData() { return m_result; }

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
	virtual void CleanUp() override;

private:
	float m_raceTime = 0.0f; //レースタイム(秒)
	float m_countDown = 3.0f; //スタート前のカウントダウン時間(秒)
	float m_startDelay = 1.0f; //スタート前の遅延時間(秒)
	bool m_raceStarted = false; //レース開始フラグ
	bool m_raceFinished = false; //レース終了フラグ

	std::vector<RacingBoat*> m_racingBoats; //レース参加ボート
	std::vector<RacingBoat*> m_rankingBoats; //順位付け用ボートリスト

	Vector2 m_boundsMin = { -215.0f, -60.0f };
	Vector2 m_boundsMax = { 215.0f, 60.0f };

	int m_lapCountToFinish = 3; //完走に必要な周回数

	static std::vector<BoatResultData> m_result;	//レース結果
};
