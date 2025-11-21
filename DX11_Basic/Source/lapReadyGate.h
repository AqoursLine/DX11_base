#pragma once

#include "gameObject.h"

class LapReadyGate : public GameObject {
public:
	LapReadyGate() = default;
	~LapReadyGate() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class RaceManager* m_raceManager = nullptr;

	bool m_isPassed = false;	//ゴール通過フラグ

	int m_topLapCount = 0;	//トップの周回数
	int m_lastLapCount = 0; //最下位の周回数
};
