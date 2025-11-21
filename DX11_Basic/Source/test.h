#pragma once

#include "gameObject.h"

class TestObject : public GameObject {
public:
	TestObject() = default;
	~TestObject() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:

	class ModelRenderer* m_modelRenderer = nullptr;
	class Box* m_box = nullptr;

	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
};
