#pragma once

#include "gameObject.h"

class RacingBoat;

class RaceManager : public GameObject {
public:
	RaceManager() = default;
	~RaceManager() = default;

	//レースタイム取得
	float GetRaceTime() const { return m_raceTime; }
	//スタート前のカウントダウン時間取得
	float GetStartDelay() const { return m_startDelay; }

	//境界ボックス取得
	Vector2 GetBoundsMin() const { return m_boundsMin; }
	Vector2 GetBoundsMax() const { return m_boundsMax; }

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;
	virtual void CleanUp() override;

private:
	float m_raceTime = 0.0f; //レースタイム(秒)
	float m_startDelay = 3.0f; //スタート前のカウントダウン時間(秒)

	std::vector<RacingBoat*> m_racingBoats; //レース参加ボート

	Vector2 m_boundsMin = { -215.0f, -60.0f };
	Vector2 m_boundsMax = { 215.0f, 60.0f };
};
