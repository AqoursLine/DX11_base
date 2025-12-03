#include "main.h"
#include "renderer.h"
#include "texture.h"
#include <filesystem>

//スタティックメンバーの初期化
std::unordered_map<std::wstring, TextureEntry*> Texture::m_textureCache;

/// <summary>
/// デストラクタ
/// </summary>
Texture::~Texture() {
	//参照カウントをデクリメント
	m_texture->refCount--;
	//参照カウントが0なら解放
	if (m_texture->refCount <= 0) {
		if (m_texture->srv) {
			m_texture->srv->Release();
			m_texture->srv = nullptr;
		}
		//マップから削除
		for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ++it) {
			if (it->second == m_texture) {
				m_textureCache.erase(it);
				break;
			}
		}
		delete m_texture;
	}
}

/// <summary>
/// テクスチャの読み込み
/// </summary>
/// <param name="fileName">ファイルパス</param>
/// <returns>読み込み成功</returns>
bool Texture::Load(const std::wstring& fileName) {
	// テクスチャのキャッシュを確認
	if (m_textureCache.count(fileName)) {
		m_texture = m_textureCache[fileName];
		m_texture->refCount++;
		return true; // キャッシュから取得成功
	}

	m_texture = new TextureEntry();

	TexMetadata metadata;
	ScratchImage scratchImg;

	// テクスチャの読み込み	
	HRESULT hrTex;

	// 拡張子チェック
	std::filesystem::path path(fileName);

	std::filesystem::path ext = path.extension();

	if (ext == L".dds") {
		hrTex = LoadFromDDSFile(fileName.c_str(), DDS_FLAGS_NONE, &metadata, scratchImg);
	}
	else {
		hrTex = LoadFromWICFile(fileName.c_str(), WIC_FLAGS_NONE, &metadata, scratchImg);
	}
	if (FAILED(hrTex)) {
		ErrorMessage(L"テクスチャの読み込みに失敗しました。", hrTex);
		return false;
	}
	hrTex = CreateShaderResourceView(RENDERER.GetDevice(), scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, &m_texture->srv);
	if (FAILED(hrTex)) {
		ErrorMessage(L"テクスチャのシェーダーリソースビューの作成に失敗しました。", hrTex);
		return false;
	}

	m_texture->refCount = 1;
	m_textureCache[fileName] = m_texture; // キャッシュに追加

	return true;
}

/// <summary>
/// シェーダーリソースビューを設定
/// </summary>
/// <param name="name">エントリ名</param>
/// <param name="srv">シェーダーリソースビュー</param>
void Texture::SetSRV(const std::wstring& name, ID3D11ShaderResourceView* srv) {
	m_texture = new TextureEntry();
	m_texture->srv = srv;
	m_texture->refCount = 1;
	m_textureCache[name] = m_texture; // キャッシュに追加
}

/// <summary>
/// テクスチャを設定
/// </summary>
/// <param name="slot">設定するスロット番号</param>
void Texture::Set(int slot) {
	auto context = RENDERER.GetDeviceContext();
	context->PSSetShaderResources(slot, 1, &m_texture->srv);
}

/// <summary>
/// テクスチャのキャッシュをすべて解放
/// </summary>
void Texture::ReleaseAll() {
	for (auto& pair : m_textureCache) {
		if (pair.second) {
			pair.second->srv->Release();
		}
		delete pair.second;
	}
	m_textureCache.clear();
}
