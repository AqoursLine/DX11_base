#pragma once

class Timer {
public:
	Timer();

	void Reset();
	void Start();
	void Stop();
	void Tick();

	double GetDeltaTime() const;
	double GetTotalTime() const;

private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_currTime;

	bool m_stoped;

};
