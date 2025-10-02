#pragma once

class Shader {
public:
	Shader() = default;
	~Shader() = default;

	void Load(const std::wstring& vertexShaderFile, const std::wstring& pixelShaderFile);
	void SetShaders() const;
	void ReleaseAll();
private:
	static std::unordered_map<std::wstring, std::pair<ID3D11VertexShader*, ID3D11InputLayout*>> m_vertexShaderCache;
	static std::unordered_map<std::wstring, ID3D11PixelShader*> m_pixelShaderCache;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
};
