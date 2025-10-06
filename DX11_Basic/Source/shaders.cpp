#include "main.h"
#include "renderer.h"
#include "shaders.h"

//静的メンバ変数の実体
std::unordered_map<std::wstring, std::pair<ID3D11VertexShader*, ID3D11InputLayout*>> VertexShader::m_loadedShaders;
std::unordered_map<std::wstring, ID3D11PixelShader*> PixelShader::m_loadedShaders;

//バーテックスシェーダー
void VertexShader::Load(std::wstring path) {
	ID3D11VertexShader* shader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

	//すでに読み込んでいるか確認
	auto it = m_loadedShaders.find(path);
	if (it != m_loadedShaders.end()) {
		m_shader = it->second;
		return;
	}

	//シェーダーの読み込み
	RENDERER.CreateVertexShader(&shader, &inputLayout, path);
	m_shader = { shader, inputLayout };

	m_loadedShaders[path] = m_shader;
}

void VertexShader::Set() {
	auto context = RENDERER.GetDeviceContext();
	context->VSSetShader(m_shader.first, nullptr, 0);
	context->IASetInputLayout(m_shader.second);
}

void VertexShader::ReleaseAll() {
	for (auto& pair : m_loadedShaders) {
		if (pair.second.first) {
			pair.second.first->Release();
			pair.second.first = nullptr;
		}
		if (pair.second.second) {
			pair.second.second->Release();
			pair.second.second = nullptr;
		}
	}
	m_loadedShaders.clear();
}

//ピクセルシェーダー
void PixelShader::Load(std::wstring path) {
	ID3D11PixelShader* shader = nullptr;
	//すでに読み込んでいるか確認
	auto it = m_loadedShaders.find(path);
	if (it != m_loadedShaders.end()) {
		m_shader = it->second;
		return;
	}
	//シェーダーの読み込み
	RENDERER.CreatePixelShader(&shader, path);
	m_shader = shader;
	m_loadedShaders[path] = m_shader;
}

void PixelShader::Set() {
	auto context = RENDERER.GetDeviceContext();
	context->PSSetShader(m_shader, nullptr, 0);
}

void PixelShader::ReleaseAll() {
	for (auto& pair : m_loadedShaders) {
		if (pair.second) {
			pair.second->Release();
			pair.second = nullptr;
		}
	}
	m_loadedShaders.clear();
}