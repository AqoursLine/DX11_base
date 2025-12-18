#include "main.h"
#include "renderer.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

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
	ComPtr<ID3D11Texture2D> backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
	m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());

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

	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	hr = m_device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルバッファの初期化に失敗しました。", hr);
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Flags = 0;
	hr = m_device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
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

	hr = m_device->CreateRasterizerState(&rasterizerDesc, m_rasterizerBack.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ラスタライザーステートの初期化に失敗しました。", hr);
		return false;
	}
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, m_rasterizerFront.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ラスタライザーステートの初期化に失敗しました。", hr);
		return false;
	}
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, m_rasterizerNone.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ラスタライザーステートの初期化に失敗しました。", hr);
		return false;
	}
	rasterizerDesc.DepthBias = 100;
	rasterizerDesc.SlopeScaledDepthBias = 0.5f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, m_rasterizerShadow.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ラスタライザーステートの初期化に失敗しました。", hr);
		return false;
	}

	m_deviceContext->RSSetState(m_rasterizerBack.Get());

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

	//有効
	hr = m_device->CreateDepthStencilState(&depthStencilDesc2, m_depthStencilStateEnable.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルステートの初期化に失敗しました。", hr);
		return false;
	}

	//無効
	depthStencilDesc2.DepthEnable = FALSE;
	depthStencilDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc2, m_depthStencilStateDisable.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"デプスステンシルステートの初期化に失敗しました。", hr);
		return false;
	}

	//深度読み取り専用
	depthStencilDesc2.DepthEnable = TRUE;
	depthStencilDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc2, m_depthStencilStateReadOnly.GetAddressOf());
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

	hr = m_device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"サンプラーステートの初期化に失敗しました。", hr);
		return false;
	}
	SetSamplerState();

	//定数バッファの初期化
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_worldBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ワールドバッファの初期化に失敗しました。", hr);
		return false;
	}
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_projectionBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"プロジェクション行列バッファの初期化に失敗しました。", hr);
		return false;
	}
	bufferDesc.ByteWidth = sizeof(VIEW_BILLBOARD_MATRIX);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_viewBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ビュー行列バッファの初期化に失敗しました。", hr);
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

	bufferDesc.ByteWidth = sizeof(LIGHT) * MAX_LIGHTS; // 最大8つのライト
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_lightBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ライトバッファの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->PSSetConstantBuffers(4, 1, m_lightBuffer.GetAddressOf());
	m_deviceContext->VSSetConstantBuffers(4, 1, m_lightBuffer.GetAddressOf());

	//カメラバッファの作成
	bufferDesc.ByteWidth = sizeof(XMFLOAT4);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_cameraBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"カメラバッファの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->VSSetConstantBuffers(5, 1, m_cameraBuffer.GetAddressOf());
	m_deviceContext->PSSetConstantBuffers(5, 1, m_cameraBuffer.GetAddressOf());

	//シェーダー用汎用プロパティバッファの作成
	bufferDesc.ByteWidth = sizeof(SHADER_PROPERTIES);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_shaderPropertiesBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"シェーダープロパティバッファの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->VSSetConstantBuffers(6, 1, m_shaderPropertiesBuffer.GetAddressOf());
	m_deviceContext->PSSetConstantBuffers(6, 1, m_shaderPropertiesBuffer.GetAddressOf());

	//ライトビュー投影行列バッファの作成
	bufferDesc.ByteWidth = sizeof(SHADOW_LIGHTS);
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_shadowLightBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"ライトビュー投影行列バッファの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->VSSetConstantBuffers(10, 1, m_shadowLightBuffer.GetAddressOf());
	m_deviceContext->PSSetConstantBuffers(10, 1, m_shadowLightBuffer.GetAddressOf());

	//シャドウマップ用のレンダーターゲット作成
	D3D11_TEXTURE2D_DESC shadowMapDesc = {};
	shadowMapDesc.Width = SHADOW_MAP_SIZE;
	shadowMapDesc.Height = SHADOW_MAP_SIZE;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = MAX_SHADOW_LIGHTS;
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> shadowMapTexture;
	hr = m_device->CreateTexture2D(&shadowMapDesc, nullptr, shadowMapTexture.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"シャドウマップ用テクスチャの初期化に失敗しました。", hr);
		return false;
	}

	//シェーダーリソースビューの作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = MAX_SHADOW_LIGHTS;

	hr = m_device->CreateShaderResourceView(shadowMapTexture.Get(), &srvDesc, m_shadowSRV.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"シャドウマップ用シェーダーリソースビューの初期化に失敗しました。", hr);
		return false;
	}

	//シャドウマップをシェーダーにセット
	m_deviceContext->PSSetShaderResources(10, 1, m_shadowSRV.GetAddressOf());

	//デプスステンシルビューの作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	for (int i = 0; i < MAX_SHADOW_LIGHTS; i++) {
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		dsvDesc.Texture2DArray.ArraySize = 1;
		ComPtr<ID3D11DepthStencilView> shadowDSV;
		hr = m_device->CreateDepthStencilView(shadowMapTexture.Get(), &dsvDesc, shadowDSV.GetAddressOf());
		if (FAILED(hr)) {
			ErrorMessage(L"シャドウマップ用デプスステンシルビューの初期化に失敗しました。", hr);
			return false;
		}
		m_shadowDSV.push_back(shadowDSV);
	}

	//比較サンプラーの作成
	D3D11_SAMPLER_DESC comparisonSamplerDesc = {};
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateSamplerState(&comparisonSamplerDesc, m_shadowSamplerState.GetAddressOf());

	if (FAILED(hr)) {
		ErrorMessage(L"比較サンプラーの初期化に失敗しました。", hr);
		return false;
	}
	m_deviceContext->PSSetSamplers(10, 1, m_shadowSamplerState.GetAddressOf());

	return true;
}
void Renderer::Finalize() {
	//レンダーターゲットの解放
	m_renderTargetSRV.clear();
	m_renderTargetRTV.clear();
}

void Renderer::BeginDraw() {
	float clearColor[4] = {0.031f, 0.91f, 0.871f, 1.0f};
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void Renderer::EndDraw() {
	m_swapChain->Present(1, 0);
}

void Renderer::SetDepthStencilState(DEPTH_MODE mode) {
	switch (mode) {
		case DEPTH_MODE::ENABLE:
			m_deviceContext->OMSetDepthStencilState(m_depthStencilStateEnable.Get(), NULL);
			break;
		case DEPTH_MODE::READ_ONLY:
			m_deviceContext->OMSetDepthStencilState(m_depthStencilStateReadOnly.Get(), NULL);
			break;
		case DEPTH_MODE::DISABLE:
			m_deviceContext->OMSetDepthStencilState(m_depthStencilStateDisable.Get(), NULL);
			break;
		default:
			break;
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

void Renderer::SetSamplerState() {
	m_deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
}

void Renderer::SetRasterizerState(RASTERIZER_MODE mode) {
	switch (mode) {
		case RASTERIZER_MODE::BACK:
			m_deviceContext->RSSetState(m_rasterizerBack.Get());
			break;
		case RASTERIZER_MODE::FRONT:
			m_deviceContext->RSSetState(m_rasterizerFront.Get());
			break;
		case RASTERIZER_MODE::NONE:
			m_deviceContext->RSSetState(m_rasterizerNone.Get());
			break;
		case RASTERIZER_MODE::SHADOW:
			m_deviceContext->RSSetState(m_rasterizerShadow.Get());
			break;
		default:
			break;
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

	//map/unmapで更新
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_worldBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &world, sizeof(world));
	m_deviceContext->Unmap(m_worldBuffer.Get(), 0);
}

void Renderer::SetViewMatrix(const XMMATRIX& viewMatrix) {
	VIEW_BILLBOARD_MATRIX viewBillboard;

	XMStoreFloat4x4(&viewBillboard.viewMatrix, XMMatrixTranspose(viewMatrix));

	XMVECTOR detach;
	XMMATRIX inverseView = XMMatrixInverse(&detach, viewMatrix);

	inverseView.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMStoreFloat4x4(&viewBillboard.billboardMatrix, XMMatrixTranspose(inverseView));

	//mapで更新
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_viewBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &viewBillboard, sizeof(viewBillboard));
	m_deviceContext->Unmap(m_viewBuffer.Get(), 0);
}

void Renderer::SetProjectionMatrix(const XMMATRIX& projectionMatrix) {
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(projectionMatrix));

	//mapで更新
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_projectionBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &projection, sizeof(projection));
	m_deviceContext->Unmap(m_projectionBuffer.Get(), 0);
}

void Renderer::SetMaterial(const MATERIAL& material) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_materialBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &material, sizeof(material));
	m_deviceContext->Unmap(m_materialBuffer.Get(), 0);
}

void Renderer::SetLights(const LIGHTS& light) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &light, sizeof(light));
	m_deviceContext->Unmap(m_lightBuffer.Get(), 0);
}

void Renderer::SetCameraData(const XMFLOAT4& camera) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &camera, sizeof(XMFLOAT4));
	m_deviceContext->Unmap(m_cameraBuffer.Get(), 0);
}

void Renderer::SetShaderProperties(const SHADER_PROPERTIES& properties) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_shaderPropertiesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &properties, sizeof(properties));
	m_deviceContext->Unmap(m_shaderPropertiesBuffer.Get(), 0);
}

void Renderer::SetShadowLights(const SHADOW_LIGHTS& shadowLights) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_shadowLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &shadowLights, sizeof(shadowLights));
	m_deviceContext->Unmap(m_shadowLightBuffer.Get(), 0);
}

void Renderer::CreateVertexShader(ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, const std::wstring& fileName) {
	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	//CSOファイルの読み込み
	hr = D3DReadFileToBlob(fileName.c_str(), shaderBlob.GetAddressOf());
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

	//シェーダーリフレクションの取得
	ComPtr<ID3D11ShaderReflection> shaderReflection;
	hr = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, &shaderReflection);
	if (FAILED(hr)) {
		ErrorMessage(L"シェーダーリフレクションの取得に失敗しました。", hr);
		shaderBlob->Release();
		return;
	}

	//入力レイアウトの作成
	D3D11_SHADER_DESC shaderDesc = {};
	shaderReflection->GetDesc(&shaderDesc);

	//入力レイアウトの要素数を取得
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
	inputElementDescs.reserve(shaderDesc.InputParameters);

	for (UINT i = 0; i < shaderDesc.InputParameters; i++) {
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc = {};
		shaderReflection->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;

		// セマンティクス名に"INSTANCE_"が含まれている場合、インスタンスデータとして扱う
		if (std::string(elementDesc.SemanticName).find("INSTANCE_") != std::string::npos) {
			elementDesc.InputSlot = 1;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;
		} else {
			elementDesc.InputSlot = 0;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
		}

		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		//データ形式の決定
		if (paramDesc.Mask == 1) {
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			}
		} else if (paramDesc.Mask <= 3) {
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			}
		} else if (paramDesc.Mask <= 7) {
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			}
		} else if (paramDesc.Mask <= 15) {
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			} else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
		}

		inputElementDescs.push_back(elementDesc);
	}

	//入力レイアウトの作成
	hr = m_device->CreateInputLayout(
		inputElementDescs.data(),
		static_cast<UINT>(inputElementDescs.size()),
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		inputLayout
	);
	if (FAILED(hr)) {
		ErrorMessage(L"入力レイアウトの作成に失敗しました。", hr);
		shaderBlob->Release();
		return;
	}
}

void Renderer::CreatePixelShader(ID3D11PixelShader** pixelShader, const std::wstring& fileName) {
	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	//CSOファイルの読み込み
	hr = D3DReadFileToBlob(fileName.c_str(), shaderBlob.GetAddressOf());
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

void Renderer::CreateComputeShader(ID3D11ComputeShader** computeShader, const std::wstring& fileName) {
	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;
	//CSOファイルの読み込み
	hr = D3DReadFileToBlob(fileName.c_str(), shaderBlob.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"CSOファイルの読み込みに失敗しました。", hr);
		return;
	}
	//シェーダーの作成
	hr = m_device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, computeShader);
	if (FAILED(hr)) {
		ErrorMessage(L"コンピュートシェーダーの作成に失敗しました。", hr);
		shaderBlob->Release();
		return;
	}
	shaderBlob->Release();
}

int Renderer::AddRenderTarget(UINT width, UINT height) {
	HRESULT hr = S_OK;
	//テクスチャ作成
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ComPtr<ID3D11Texture2D> texture;
	hr = m_device->CreateTexture2D(&textureDesc, nullptr, texture.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"レンダーターゲット用テクスチャの作成に失敗しました。", hr);
		return 0;
	}
	//レンダーターゲットビュー作成
	ComPtr<ID3D11RenderTargetView> rtv;
	hr = m_device->CreateRenderTargetView(texture.Get(), nullptr, rtv.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"レンダーターゲットビューの作成に失敗しました。", hr);
		return 0;
	}
	m_renderTargetRTV.push_back(rtv);
	//シェーダーリソースビュー作成
	ComPtr<ID3D11ShaderResourceView> srv;
	hr = m_device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"シェーダーリソースビューの作成に失敗しました。", hr);
		return 0;
	}
	m_renderTargetSRV.push_back(srv);

	//サイズ保存
	m_renderTargetSizes.push_back({ static_cast<float>(width), static_cast<float>(height) });

	//インデックスを返す
	return static_cast<int>(m_renderTargetRTV.size() - 1);
}

void Renderer::SetRenderTarget(int index) {
	if (index < 0) {
		//ビューポートの設定
		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(SCREEN_WIDTH);
		viewport.Height = static_cast<float>(SCREEN_HEIGHT);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		m_deviceContext->RSSetViewports(1, &viewport);

		m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	} else {
		if (index >= static_cast<int>(m_renderTargetRTV.size())) {
			return;
		}
		//レンダリングターゲットのクリア
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_deviceContext->ClearRenderTargetView(m_renderTargetRTV[index].Get(), clearColor);

		//デプスステンシルビューのクリア
		m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		//ビューポートの設定
		D3D11_VIEWPORT viewport = {};
		viewport.Width = m_renderTargetSizes[index].x;
		viewport.Height = m_renderTargetSizes[index].y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		m_deviceContext->RSSetViewports(1, &viewport);

		//レンダーターゲットの設定
		m_deviceContext->OMSetRenderTargets(1, m_renderTargetRTV[index].GetAddressOf(), m_depthStencilView.Get());
	}
}

void Renderer::ClearRenderTarget(int index, float r, float g, float b, float a) {
	if (index < 0 || index >= static_cast<int>(m_renderTargetRTV.size())) {
		return;
	}
	float clearColor[4] = { r, g, b, a };
	m_deviceContext->ClearRenderTargetView(m_renderTargetRTV[index].Get(), clearColor);
}

void Renderer::SetDefaultRenderTarget() {
	//ビューポートの設定
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_deviceContext->RSSetViewports(1, &viewport);

	float clearColor[4] = { 0.031f, 0.91f, 0.871f, 1.0f };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

void Renderer::ClearDefaultRenderTarget(float r, float g, float b, float a) {
	float clearColor[4] = { r, g, b, a };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}

ID3D11ShaderResourceView* Renderer::GetRenderTargetSRV(int index) {
	if (index < 0 || index >= static_cast<int>(m_renderTargetSRV.size())) {
		return nullptr;
	}
	return m_renderTargetSRV[index].Get();
}

void Renderer::SetShadowMapAsRenderTarget(int index) {
	if (index < 0 || index >= static_cast<int>(m_shadowDSV.size())) {
		return;
	}

	//ビューポートの設定
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(SHADOW_MAP_SIZE);
	viewport.Height = static_cast<float>(SHADOW_MAP_SIZE);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_deviceContext->RSSetViewports(1, &viewport);

	//シャドウマップ用デプスステンシルビューをレンダーターゲットに設定
	m_deviceContext->OMSetRenderTargets(0, nullptr, m_shadowDSV[index].Get());

}

void Renderer::ClearShadowMap(int index) {
	if (index < 0 || index >= static_cast<int>(m_shadowDSV.size())) {
		return;
	}
	//深度ステンシルビューのクリア
	m_deviceContext->ClearDepthStencilView(m_shadowDSV[index].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::SetShadowMapSRV() {
	m_deviceContext->PSSetShaderResources(10, 1, m_shadowSRV.GetAddressOf());
}
