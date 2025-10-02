#include "main.h"
#include "renderer.h"
#include "shader.h"

//静的メンバ変数の初期化
std::unordered_map<std::wstring, std::pair<ID3D11VertexShader*, ID3D11InputLayout*>> Shader::m_vertexShaderCache;
std::unordered_map<std::wstring, ID3D11PixelShader*> Shader::m_pixelShaderCache;

void Shader::Load(const std::wstring& vertexShaderFile, const std::wstring& pixelShaderFile) {
	//シェーダーの確認
	auto vertexIt = m_vertexShaderCache.find(vertexShaderFile);
	if (vertexIt != m_vertexShaderCache.end()) {
		m_vertexShader = vertexIt->second.first;
		m_inputLayout = vertexIt->second.second;
	} else {
		RENDERER.CreateVertexShader(&m_vertexShader, &m_inputLayout, vertexShaderFile);
		m_vertexShaderCache[vertexShaderFile] = { m_vertexShader, m_inputLayout };
	}
	auto pixelIt = m_pixelShaderCache.find(pixelShaderFile);
	if (pixelIt != m_pixelShaderCache.end()) {
		m_pixelShader = pixelIt->second;
	} else {
		RENDERER.CreatePixelShader(&m_pixelShader, pixelShaderFile);
		m_pixelShaderCache[pixelShaderFile] = m_pixelShader;
	}
}

void Shader::SetShaders() const {
	//シェーダーの設定
	RENDERER.GetDeviceContext()->IASetInputLayout(m_inputLayout);
	RENDERER.GetDeviceContext()->VSSetShader(m_vertexShader, nullptr, 0);
	RENDERER.GetDeviceContext()->PSSetShader(m_pixelShader, nullptr, 0);
}

void Shader::ReleaseAll() {
	for (auto& pair : m_vertexShaderCache) {
		if (pair.second.first) {
			pair.second.first->Release();
			pair.second.first = nullptr;
		}
		if (pair.second.second) {
			pair.second.second->Release();
			pair.second.second = nullptr;
		}
	}
	m_vertexShaderCache.clear();
	for (auto& pair : m_pixelShaderCache) {
		if (pair.second) {
			pair.second->Release();
			pair.second = nullptr;
		}
	}
	m_pixelShaderCache.clear();
}
