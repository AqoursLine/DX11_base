#include "transition.h"

bool Transition::Initialize() {
	return true;
}
void Transition::Finalize() {
}

void Transition::Update(double deltaTime) {
	m_oldIsInTransitionFinished = m_isInTransitionFinished;
	m_oldIsOutTransitionFinished = m_isOutTransitionFinished;

	switch (m_state) {
		case TransitionState::In:
			UpdateInTransition(deltaTime);
			break;
		case TransitionState::Out:
			UpdateOutTransition(deltaTime);
			break;
		case TransitionState::Loading:
			UpdateTransition(deltaTime);
			break;
		case TransitionState::None:
		default:
			break;
	}
}

void Transition::Draw() {
}

void Transition::StartInTransition() {
	m_state = TransitionState::In;
	m_isInTransitionFinished = false;
	m_oldIsInTransitionFinished = false;
}

void Transition::StartOutTransition() {
	m_state = TransitionState::Out;
	m_isOutTransitionFinished = false;
	m_oldIsOutTransitionFinished = false;
}
