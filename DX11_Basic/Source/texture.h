#pragma once


class Texture {
public:
	Texture() = default;
	~Texture() = default;

	bool Load(std::wstring fileName);

	static void ReleaseAll();

	ID3D11ShaderResourceView* GetTexture() const { return m_texture; }
	ID3D11ShaderResourceView** GetTextureAddress() { return &m_texture; }
private:
	ID3D11ShaderResourceView* m_texture;

	static std::unordered_map<std::wstring, ID3D11ShaderResourceView*> m_textureCache;
};
