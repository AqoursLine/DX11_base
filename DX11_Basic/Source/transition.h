#pragma once
#include "main.h"

enum class TransitionState {
	None,
	In,
	Out,
	Loading,
};

class Transition {
public:
	virtual bool Initialize();
	virtual void Finalize();
	virtual void Update(double deltaTime);
	virtual void Draw();

	void StartInTransition();
	void StartOutTransition();

	bool IsInTransitionFinished() const { return m_isInTransitionFinished; }
	bool IsInTransitionFinishedTriggered() const {
		return m_isInTransitionFinished && !m_oldIsInTransitionFinished;
	}
	bool IsOutTransitionFinished() const { return m_isOutTransitionFinished; }
	bool IsOutTransitionFinishedTriggered() const {
		return m_isOutTransitionFinished && !m_oldIsOutTransitionFinished;
	}

	TransitionState GetState() const { return m_state; }

protected:
	void SetInTransitionFinished(bool isFinished) {
		m_isInTransitionFinished = isFinished;
		m_state = TransitionState::None;
	}

	void SetOutTransitionFinished(bool isFinished) {
		m_isOutTransitionFinished = isFinished;
		m_state = TransitionState::Loading;
	}

	virtual void UpdateInTransition(double deltaTime) = 0;
	virtual void UpdateTransition(double deltaTime) = 0;
	virtual void UpdateOutTransition(double deltaTime) = 0;

private:
	TransitionState m_state = TransitionState::None;

	bool m_isInTransitionFinished = false;
	bool m_oldIsInTransitionFinished = false;
	bool m_isOutTransitionFinished = false;
	bool m_oldIsOutTransitionFinished = false;
};
