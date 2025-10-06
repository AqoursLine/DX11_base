#pragma once

class VertexShader {
public:
	VertexShader() = default;
	virtual ~VertexShader() = default;

	void Load(std::wstring path);
	void Set();

	static void ReleaseAll();
private:
	static std::unordered_map<std::wstring, std::pair<ID3D11VertexShader*, ID3D11InputLayout*>> m_loadedShaders;
	std::pair<ID3D11VertexShader*, ID3D11InputLayout*> m_shader = { nullptr, nullptr };
};

class PixelShader {
public:
	PixelShader() = default;
	virtual ~PixelShader() = default;
	void Load(std::wstring path);
	void Set();
	static void ReleaseAll();
private:
	static std::unordered_map<std::wstring, ID3D11PixelShader*> m_loadedShaders;
	ID3D11PixelShader* m_shader = nullptr;
};
