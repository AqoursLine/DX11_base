#include "main.h"
#include "renderer.h"
#include "texture.h"

//スタティックメンバーの初期化
std::unordered_map<std::wstring, ID3D11ShaderResourceView*> Texture::m_textureCache;

bool Texture::Load(std::wstring fileName) {
	// テクスチャのキャッシュを確認
	if (m_textureCache.count(fileName)) {
		m_texture = m_textureCache[fileName];
		return true; // キャッシュから取得成功
	}

	TexMetadata metadata;
	ScratchImage scratchImg;
	HRESULT hrTex = LoadFromWICFile(fileName.c_str(), WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(hrTex)) {
		ErrorMessage(L"テクスチャの読み込みに失敗しました。", hrTex);
		return false;
	}
	CreateShaderResourceView(RENDERER.GetInstance().GetDevice(), scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, &m_texture);
	if (FAILED(hrTex)) {
		ErrorMessage(L"テクスチャのシェーダーリソースビューの作成に失敗しました。", hrTex);
		return false;
	}
	m_textureCache[fileName] = m_texture; // キャッシュに追加

	return true;
}

void Texture::Set(int slot) {
	auto context = RENDERER.GetDeviceContext();
	context->PSSetShaderResources(slot, 1, &m_texture);
}

void Texture::ReleaseAll() {
	for (auto& pair : m_textureCache) {
		if (pair.second) {
			pair.second->Release();
		}
	}
	m_textureCache.clear();
}
