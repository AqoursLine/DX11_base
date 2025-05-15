#include "renderer.h"
ComPtr<IDXGISwapChain>		Renderer::m_swapChain;
ComPtr<ID3D11Device>		Renderer::m_device;
ComPtr<ID3D11DeviceContext>	Renderer::m_deviceContext;
D3D_FEATURE_LEVEL			Renderer::m_featureLevel = D3D_FEATURE_LEVEL_11_0;

bool Renderer::Initialize(HWND hWnd) {
	HRESULT he = S_OK;

	DXGI_SWAP_CHAIN_DESC scd{};
	scd.BufferCount = 1;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;

	he = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&scd,
		m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(),
		&m_featureLevel,
		m_deviceContext.GetAddressOf()
	);

}
