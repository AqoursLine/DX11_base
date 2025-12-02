#include "main.h"
#include "renderer.h"
#include "shaders.h"

//静的メンバ変数の実体
std::unordered_map<std::wstring, VShader> VertexShader::m_loadedShaders;
std::unordered_map<std::wstring, PShader> PixelShader::m_loadedShaders;
std::unordered_map<std::wstring, CShader> ComputeShader::m_loadedShaders;

//==============================================================================
//バーテックスシェーダー
//==============================================================================

/// <summary>
/// デストラクタ
/// </summary>
VertexShader::~VertexShader() {
	//参照カウントをデクリメント
	m_shader->refCount--;
	//参照カウントが0なら解放
	if (m_shader->refCount <= 0) {
		if (m_shader->shader) {
			m_shader->shader->Release();
			m_shader->shader = nullptr;
		}
		if (m_shader->layout) {
			m_shader->layout->Release();
			m_shader->layout = nullptr;
		}
		//マップから削除
		for (auto it = m_loadedShaders.begin(); it != m_loadedShaders.end(); ++it) {
			if (it->second.shader == m_shader->shader) {
				m_loadedShaders.erase(it);
				break;
			}
		}
	}
}

/// <summary>
/// バーテックスシェーダーの読み込み
/// </summary>
/// <param name="path">シェーダーのパス</param>
void VertexShader::Load(const std::wstring& path) {
	ID3D11VertexShader* shader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

	//すでに読み込んでいるか確認
	auto it = m_loadedShaders.find(path);
	if (it != m_loadedShaders.end()) {
		m_shader = &(it->second);
		m_shader->refCount++;
		return;
	}

	//シェーダーの読み込み
	RENDERER.CreateVertexShader(&shader, &inputLayout, path);
	
	m_loadedShaders[path] = VShader { 1, shader, inputLayout };
	
	m_shader = &m_loadedShaders[path];
}

/// <summary>
/// バーテックスシェーダーの設定
/// </summary>
void VertexShader::Set() {
	auto context = RENDERER.GetDeviceContext();
	context->VSSetShader(m_shader->shader, nullptr, 0);
	context->IASetInputLayout(m_shader->layout);
}

/// <summary>
/// バーテックスシェーダーの解放
/// </summary>
void VertexShader::ReleaseAll() {
	for (auto& pair : m_loadedShaders) {
		if (pair.second.shader) {
			pair.second.shader->Release();
			pair.second.shader = nullptr;
		}
		if (pair.second.layout) {
			pair.second.layout->Release();
			pair.second.layout = nullptr;
		}
	}
	m_loadedShaders.clear();
}

//==============================================================================
//ピクセルシェーダー
//==============================================================================

/// <summary>
/// デストラクタ
/// </summary>
PixelShader::~PixelShader() {
	//参照カウントをデクリメント
	m_shader->refCount--;
	//参照カウントが0なら解放
	if (m_shader->refCount <= 0) {
		if (m_shader->shader) {
			m_shader->shader->Release();
			m_shader->shader = nullptr;
		}
		//マップから削除
		for (auto it = m_loadedShaders.begin(); it != m_loadedShaders.end(); ++it) {
			if (it->second.shader == m_shader->shader) {
				m_loadedShaders.erase(it);
				break;
			}
		}
	}
}

/// <summary>
/// ピクセルシェーダーの読み込み
/// </summary>
/// <param name="path">シェーダーのパス</param>
void PixelShader::Load(const std::wstring& path) {
	ID3D11PixelShader* shader = nullptr;
	//すでに読み込んでいるか確認
	auto it = m_loadedShaders.find(path);
	if (it != m_loadedShaders.end()) {
		m_shader = &(it->second);
		m_shader->refCount++;
		return;
	}
	//シェーダーの読み込み
	RENDERER.CreatePixelShader(&shader, path);
	m_loadedShaders[path] = PShader { 1, shader };
	m_shader = &m_loadedShaders[path];
}

/// <summary>
/// ピクセルシェーダーの設定
/// </summary>
void PixelShader::Set() {
	auto context = RENDERER.GetDeviceContext();
	context->PSSetShader(m_shader->shader, nullptr, 0);
}

/// <summary>
/// ピクセルシェーダーの解放
/// </summary>
void PixelShader::ReleaseAll() {
	for (auto& pair : m_loadedShaders) {
		if (pair.second.shader) {
			pair.second.shader->Release();
			pair.second.shader = nullptr;
		}
	}
	m_loadedShaders.clear();
}

//==============================================================================
//コンピュートシェーダー
//==============================================================================

/// <summary>
/// デストラクタ
/// </summary>
ComputeShader::~ComputeShader() {
	//参照カウントをデクリメント
	m_shader->refCount--;
	//参照カウントが0なら解放
	if (m_shader->refCount <= 0) {
		if (m_shader->shader) {
			m_shader->shader->Release();
			m_shader->shader = nullptr;
		}
		//マップから削除
		for (auto it = m_loadedShaders.begin(); it != m_loadedShaders.end(); ++it) {
			if (it->second.shader == m_shader->shader) {
				m_loadedShaders.erase(it);
				break;
			}
		}
	}
}

/// <summary>
/// コンピュートシェーダーの読み込み
/// </summary>
/// <param name="path">シェーダーのパス</param>
void ComputeShader::Load(const std::wstring& path) {
	ID3D11ComputeShader* shader = nullptr;
	//すでに読み込んでいるか確認
	auto it = m_loadedShaders.find(path);
	if (it != m_loadedShaders.end()) {
		m_shader = &(it->second);
		m_shader->refCount++;
		return;
	}
	//シェーダーの読み込み
	RENDERER.CreateComputeShader(&shader, path);
	m_loadedShaders[path] = CShader { 1, shader };
	m_shader = &m_loadedShaders[path];
}

/// <summary>
/// コンピュートシェーダーの設定
/// </summary>
void ComputeShader::Set() {
	auto context = RENDERER.GetDeviceContext();
	context->CSSetShader(m_shader->shader, nullptr, 0);
}

/// <summary>
/// コンピュートシェーダーの解放
/// </summary>
void ComputeShader::ReleaseAll() {
	for (auto& pair : m_loadedShaders) {
		if (pair.second.shader) {
			pair.second.shader->Release();
			pair.second.shader = nullptr;
		}
	}
	m_loadedShaders.clear();
}