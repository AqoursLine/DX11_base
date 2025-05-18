#include "../main.h"
#include "renderer.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

Renderer* Renderer::s_instance = nullptr;

bool Renderer::Initialize(HWND hWnd) {
	HRESULT hr = S_OK;

	m_featureLevel = D3D_FEATURE_LEVEL_11_0;

	//デバイススワップチェインの初期化
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(),
		&m_featureLevel,
		m_deviceContext.GetAddressOf()
	);

	if (FAILED(hr)) {
		ErrorMessage(L"デバイススワップチェインの初期化に失敗しました。", hr);
		return false;
	}

	//レンダーターゲットビューの初期化
	ID3D11Texture2D* backBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	m_device->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetView.GetAddressOf());
	backBuffer->Release();

	if (FAILED(hr)) {
		ErrorMessage(L"レンダーターゲットビューの初期化に失敗しました。", hr);
		return false;
	}

	//デプスステンシルビューの初期化
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = SCREEN_WIDTH;
	depthStencilDesc.Height = SCREEN_HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
	depthStencilDesc.SampleDesc = swapChainDesc.SampleDesc;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	ID3D11Texture2D* depthStencilBuffer = nullptr;
	hr = m_device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルバッファの初期化に失敗しました。", hr);
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = m_device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
	depthStencilBuffer->Release();
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルビューの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	//ビューポートの初期化
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_deviceContext->RSSetViewports(1, &viewport);

	//ラスタライザーステートの初期化
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;

	ID3D11RasterizerState* rasterizerState = nullptr;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	if (FAILED(hr)) {
		ErrorMessage(L"ラスタライザーステートの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->RSSetState(rasterizerState);
	//rasterizerState->Release();

	//ブレンドステートの初期化
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ブレンドステートの初期化に失敗しました。", hr);
		return false;
	}
	blendDesc.AlphaToCoverageEnable = TRUE;
	hr = m_device->CreateBlendState(&blendDesc, m_blendStateATC.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ATCブレンドステートの初期化に失敗しました。", hr);
		return false;
	}

	//ブレンドステートの設定
	float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_deviceContext->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);

	//デプスステンシルステートの初期化
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc2 = {};
	depthStencilDesc2.DepthEnable = TRUE;
	depthStencilDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc2.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc2.StencilEnable = FALSE;

	hr = m_device->CreateDepthStencilState(&depthStencilDesc2, m_depthStencilStateEnable.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルステートの初期化に失敗しました。", hr);
		return false;
	}

	depthStencilDesc2.DepthEnable = FALSE;
	depthStencilDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc2, m_depthStencilStateDisable.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルステートの初期化に失敗しました。", hr);
		return false;
	}

	m_deviceContext->OMSetDepthStencilState(m_depthStencilStateEnable.Get(), NULL);

	//サンプラーステートの初期化
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* samplerState = nullptr;
	hr = m_device->CreateSamplerState(&samplerDesc, &samplerState);
	if (FAILED(hr)) {
		ErrorMessage(L"サンプラーステートの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->PSSetSamplers(0, 1, &samplerState);
//	samplerState->Release();
	

	//定数バッファの初期化
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_worldBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ワールドバッファの初期化に失敗しました。", hr);
		return false;
	}
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_viewBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ビュー行列バッファの初期化に失敗しました。", hr);
		return false;
	}
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_projectionBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"プロジェクション行列バッファの初期化に失敗しました。", hr);
		return false;
	}

	m_deviceContext->VSSetConstantBuffers(0, 1, m_worldBuffer.GetAddressOf());
	m_deviceContext->VSSetConstantBuffers(1, 1, m_viewBuffer.GetAddressOf());
	m_deviceContext->VSSetConstantBuffers(2, 1, m_projectionBuffer.GetAddressOf());

	bufferDesc.ByteWidth = sizeof(MATERIAL);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_materialBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"マテリアルバッファの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->PSSetConstantBuffers(3, 1, m_materialBuffer.GetAddressOf());
	m_deviceContext->VSSetConstantBuffers(3, 1, m_materialBuffer.GetAddressOf());

	bufferDesc.ByteWidth = sizeof(LIGHT);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_lightBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ライトバッファの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->PSSetConstantBuffers(4, 1, m_lightBuffer.GetAddressOf());
	m_deviceContext->VSSetConstantBuffers(4, 1, m_lightBuffer.GetAddressOf());

	//ライト初期化
	LIGHT light = {};
	light.enable = true;
	light.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	light.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetLight(light);

	//マテリアル初期化
	MATERIAL material = {};
	material.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	return true;
}

void Renderer::Finalize() {
}

void Renderer::BeginDraw() {
	float clearColor[4] = {0.031f, 0.91f, 0.871f, 1.0f};
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void Renderer::EndDraw() {
	m_swapChain->Present(1, 0);
}

void Renderer::SetDepthStencilState(bool enable) {
	if (enable) {
		m_deviceContext->OMSetDepthStencilState(m_depthStencilStateEnable.Get(), NULL);
	} else {
		m_deviceContext->OMSetDepthStencilState(m_depthStencilStateDisable.Get(), NULL);
	}
}

void Renderer::SetATCEnable(bool enable) {
	float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	if (enable) {
		m_deviceContext->OMSetBlendState(m_blendStateATC.Get(), blendFactor, 0xffffffff);
	} else {
		m_deviceContext->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);
	}
}

void Renderer::Set2DMatrix() {
	SetWorldMatrix(XMMatrixIdentity());
	SetViewMatrix(XMMatrixIdentity());

	XMMATRIX poejection;
	poejection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	SetProjectionMatrix(poejection);
}

void Renderer::SetWorldMatrix(const XMMATRIX& worldMatrix) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(worldMatrix));
	m_deviceContext->UpdateSubresource(m_worldBuffer.Get(), 0, nullptr, &world, 0, 0);
}

void Renderer::SetViewMatrix(const XMMATRIX& viewMatrix) {
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(viewMatrix));
	m_deviceContext->UpdateSubresource(m_viewBuffer.Get(), 0, nullptr, &view, 0, 0);
}

void Renderer::SetProjectionMatrix(const XMMATRIX& projectionMatrix) {
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(projectionMatrix));
	m_deviceContext->UpdateSubresource(m_projectionBuffer.Get(), 0, nullptr, &projection, 0, 0);
}

void Renderer::SetMaterial(const MATERIAL& material) {
	m_deviceContext->UpdateSubresource(m_materialBuffer.Get(), 0, nullptr, &material, 0, 0);
}

void Renderer::SetLight(const LIGHT& light) {
	m_deviceContext->UpdateSubresource(m_lightBuffer.Get(), 0, nullptr, &light, 0, 0);
}

void Renderer::CreateVertexShader(ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, std::wstring fileName) {
	HRESULT hr = S_OK;
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	//CSOファイルの読み込み
	hr = D3DReadFileToBlob(fileName.c_str(), &shaderBlob);
	if (FAILED(hr)) {
		ErrorMessage(L"CSOファイルの読み込みに失敗しました。", hr);
		return;
	}

	//シェーダーの作成
	hr = m_device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, vertexShader);
	if (FAILED(hr)) {
		ErrorMessage(L"頂点シェーダーの作成に失敗しました。", hr);
		shaderBlob->Release();
		return;
	}

	//入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = m_device->CreateInputLayout(layout, numElements, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), inputLayout);
	if (FAILED(hr)) {
		ErrorMessage(L"入力レイアウトの作成に失敗しました。", hr);
		shaderBlob->Release();
		return;
	}

	shaderBlob->Release();
}

void Renderer::CreatePixelShader(ID3D11PixelShader** pixelShader, std::wstring fileName) {
	HRESULT hr = S_OK;
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	//CSOファイルの読み込み
	hr = D3DReadFileToBlob(fileName.c_str(), &shaderBlob);
	if (FAILED(hr)) {
		ErrorMessage(L"CSOファイルの読み込みに失敗しました。", hr);
		return;
	}

	//シェーダーの作成
	hr = m_device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, pixelShader);
	if (FAILED(hr)) {
		ErrorMessage(L"ピクセルシェーダーの作成に失敗しました。", hr);
		shaderBlob->Release();
		return;
	}

	shaderBlob->Release();
}
