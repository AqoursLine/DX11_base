#pragma once

#include "gameObject.h"

class StartGate : public GameObject {
public:
	StartGate() = default;
	~StartGate() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

};
