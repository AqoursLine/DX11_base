#pragma once

#include "gameObject.h"

class SkyDome : public GameObject {
public:
	SkyDome() = default;
	~SkyDome() = default;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;

private:
	class ModelRenderer* m_model = nullptr;

	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

};
