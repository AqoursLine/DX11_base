#pragma once

#include "gameObject.h"

class FieldObject : public GameObject {
public:
	FieldObject() = default;
	~FieldObject() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;
private:
	class Field* m_field = nullptr;

};
