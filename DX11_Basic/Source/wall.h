#pragma once

#include "gameObject.h"

class Wall : public GameObject {
public:
	Wall() = default;
	~Wall() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Box* m_model = nullptr;
	class VertexShader* m_vs = nullptr;
	class PixelShader* m_ps = nullptr;

};
