#pragma once

struct VShader {
	int refCount;
	ID3D11VertexShader* shader;
	ID3D11InputLayout* layout;
};

class VertexShader {
public:
	VertexShader() = default;
	virtual ~VertexShader();

	void Load(std::wstring path);
	void Set();

	static void ReleaseAll();
private:
	static std::unordered_map<std::wstring, VShader> m_loadedShaders;
	VShader* m_shader = nullptr;
};

struct PShader {
	int refCount;
	ID3D11PixelShader* shader;
};

class PixelShader {
public:
	PixelShader() = default;
	virtual ~PixelShader();
	void Load(std::wstring path);
	void Set();
	static void ReleaseAll();
private:
	static std::unordered_map<std::wstring, PShader> m_loadedShaders;
	PShader* m_shader = nullptr;
};
