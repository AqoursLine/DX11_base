#pragma once

struct TextureEntry {
	int refCount;
	ID3D11ShaderResourceView* srv;
};

class Texture {
public:
	Texture() = default;
	~Texture();

	bool Load(std::wstring fileName);

	void SetSRV(std::wstring name, ID3D11ShaderResourceView* srv);
	ID3D11ShaderResourceView* GetSRV() { return m_texture->srv; }

	static void ReleaseAll();

	void Set(int slot = 0);


private:
	TextureEntry* m_texture;

	static std::unordered_map<std::wstring, TextureEntry*> m_textureCache;
};
