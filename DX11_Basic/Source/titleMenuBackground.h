#pragma once

#include "gameObject.h"

class TitleManuBackground : public GameObject {
public:
	TitleManuBackground() = default;
	~TitleManuBackground() = default;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
private:
	ComPtr<ID3D11Buffer> m_vertexBuffer;

	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	XMFLOAT4 m_color;

};
