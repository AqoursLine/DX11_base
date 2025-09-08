#include "fieldObject.h"
#include "field.h"

bool FieldObject::Initialize() {
	m_field = new Field();
	if (!m_field->Initialize(L"Asset\\Texture\\race.jpg")) {
		return false;
	}

	m_scale = {256.0, 1.0f, 160.0};
	m_position = { 0.0f, -2.0f, 0.0f };

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
