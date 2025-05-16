#include "manager.h"
#include "DX11/renderer.h"

bool Manager::m_isFinished = false;

Manager::Manager() {
}

Manager::~Manager() {
}

bool Manager::Initialize() {
	m_isFinished = false;


	return false;
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


