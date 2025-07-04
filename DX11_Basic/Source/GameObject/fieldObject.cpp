#include "fieldObject.h"
#include "Component/field.h"
#include "System/system.h"
#include "System/physics.h"

bool FieldObject::Initialize() {
	m_field = new Field();
	if (!m_field->Initialize(L"Asset\\Texture\\大崎甜花_ゲーミング.png")) {
		return false;
	}

	m_scale = { 16.0f, 1.0f, 9.0f };

	// 物理オブジェクトの作成
	m_body = SYSTEM.GetPhysics()->CreateStaticBox(
		m_position,
		m_rotation,
		m_scale
	);

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
