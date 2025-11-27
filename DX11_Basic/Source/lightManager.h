#pragma once

#include "gameObject.h"

class Light;

class LightManager : public GameObject {
public:
	LightManager() = default;
	~LightManager() = default;
protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
private:
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	std::vector<Light*> m_lights;
	int m_lightCount = 0;
};
