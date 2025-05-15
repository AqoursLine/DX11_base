#include "manager.h"

Manager::Manager() {
}

Manager::~Manager() {
}

void Manager::Initialize() {
}

void Manager::Finalize() {
}

bool Manager::Update(double dt) {

	if (m_isFinished) {
		return true;
	}

	return false;
}

void Manager::Draw() const {
}

void Manager::CleanUp() {
}


