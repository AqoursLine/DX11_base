#pragma once

class Texture {
public:
	Texture() = default;
	~Texture() = default;

	bool Load(std::wstring fileName);

	static void ReleaseAll();

	void Set(int slot = 0);

private:
	ID3D11ShaderResourceView* m_texture;

	static std::unordered_map<std::wstring, ID3D11ShaderResourceView*> m_textureCache;
};
