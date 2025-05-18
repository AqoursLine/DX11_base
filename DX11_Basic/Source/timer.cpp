#include "main.h"
#include "timer.h"

Timer::Timer() : m_secondsPerCount(0.0), m_deltaTime(0.0),
m_baseTime(0),m_pausedTime(0),m_stopTime(0),
m_prevTime(0),m_currTime(0),m_stoped(false){
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0 / (double)countsPerSec;
}

void Timer::Reset() {
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_stoped = false;
	m_pausedTime = 0;
}

void Timer::Start() {
	if (m_stoped) {
		__int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_stoped = false;
	}
}

void Timer::Stop() {
	if (!m_stoped) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime;
		m_stoped = true;
	}
}

void Timer::Tick() {
	if (m_stoped) {
		m_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;

	m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;
	m_prevTime = m_currTime;

	if (m_deltaTime < 0.0) {
		m_deltaTime = 0.0;
	}
}

double Timer::GetDeltaTime() const {
	return m_deltaTime;
}

double Timer::GetTotalTime() const {
	if (m_stoped) {
		return ((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount;
	} else {
		return ((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount;
	}
}
