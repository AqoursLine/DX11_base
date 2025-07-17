#include "fieldObject.h"
#include "field.h"

bool FieldObject::Initialize() {
	m_field = new Field();
	if (!m_field->Initialize(L"Asset\\Texture\\‘åè“[‰Ô_ƒQ[ƒ~ƒ“ƒO.png")) {
		return false;
	}

	m_scale = {0.01f, 1.0f, 0.01f};

	return true;
}

void FieldObject::Finalize() {
	if (m_field) {
		m_field->Finalize();
		delete m_field;
		m_field = nullptr;
	}
}

void FieldObject::Update(double deltaTime) {
}

void FieldObject::Draw() const {
	m_field->Draw(m_position, m_rotation, m_scale);

}
