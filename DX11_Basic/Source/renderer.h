#pragma once
#include "main.h"

class Renderer {
public:
	static bool Initialize(HWND hWnd);
	static void Finalize();

private:
	static ComPtr<IDXGISwapChain> m_swapChain;
	static ComPtr<ID3D11Device> m_device;
	static ComPtr<ID3D11DeviceContext> m_deviceContext;
	static D3D_FEATURE_LEVEL m_featureLevel;
};
