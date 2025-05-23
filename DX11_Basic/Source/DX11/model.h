#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>

//モデル用マテリアル構造体
struct MODEL_MATERIAL {
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 ambient;
	float shininess;
	ID3D11ShaderResourceView* texture;
	std::string texturePath;

};

//メッシュ構造体
struct MESH {
	const aiScene* scene = nullptr;

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;

	UINT numIndices;
	MODEL_MATERIAL material;
};

//テクスチャキャッシュエントリ
struct TEXTURE_CACHE_ENTRY {
	ID3D11ShaderResourceView* srv;
	int referenceCount;

	TEXTURE_CACHE_ENTRY() : srv(nullptr), referenceCount(0) {}
	TEXTURE_CACHE_ENTRY(ID3D11ShaderResourceView* srvPtr) : srv(srvPtr), referenceCount(1) {}
};

class Model {
public:
	Model() = default;
	~Model() = default;

	bool LoadModelFBX(const std::string& fileName);
	void ReleaseModel();

	void Draw() const;

	static void CleaeCache();

private:
	//ノード処理
	void ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory);
	//メッシュ処理
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& modelDirectory);

	//マテリアルロード
	MATERIAL LoadMaterial(aiMaterial* material, const aiScene* scene, const std::string& modeDirectory);
	//組み込みテクスチャをロード
	ID3D11ShaderResourceView* LoadEmbeddedTexture(const aiTexture* embeddedTexture, int textureIndex);
	//キャッシュを使用してテクスチャをロード
	ID3D11ShaderResourceView* LoadTexture(const std::string& texturePath);
	//テクスチャロード用関数
	ID3D11ShaderResourceView* LoadTextureFromFile(const std::string& texturePath);


	std::vector<MESH> m_meshes;
	std::string m_directory;

	//テクスチャキャッシュ
	static std::unordered_map<std::string, TEXTURE_CACHE_ENTRY> m_textureCache;

	//テクスチャパスリスト
	std::vector<std::string> m_usedTexturePaths;
};
