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
	//テクスチャ
	class Texture* m_texture = nullptr;
	// Shader
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
};
