#include "../main.h"
#include "renderer.h"
#include "model.h"

//assimpでFBXを読み込むための設定
bool Model::LoadModelFBX(const std::string& fileName) {
	//ファイルパスからディレクトリパスを抽出
	m_directory = fileName.substr(0, fileName.find_last_of("/\\"));

	//インポーター
	Assimp::Importer importer;

	//モデルの読み込みオプション
	unsigned int flags;
	flags = aiProcess_Triangulate |				//全ての図形を三角形化
			aiProcess_GenSmoothNormals |		//法線ベクトルを生成
			aiProcess_FlipUVs |					//UVをD3D向けに反転
			aiProcess_CalcTangentSpace |		//接戦空間を計算
			aiProcess_JoinIdenticalVertices;	//頂点を結合

	//モデルを読み込む
	const aiScene* scene = importer.ReadFile(fileName, flags);

	//読み込み失敗
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		return false;
	}

	//ルートノードから処理を開始
	ProcessNode(scene->mRootNode, scene, m_directory);
}

//メッシュのメモリを解放する関数
void Model::ReleaseModel() {

}

///ノードを処理する関数
void Model::ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory) {
	//ノードのメッシュを処理する
	for (UINT i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, modelDirectory);
	}

	//ノードの子ノードを処理する
	for (UINT i = 0; i < node->mNumChildren; i++) {
		ProcessNode(node->mChildren[i], scene, modelDirectory);
	}
}

//メッシュを処理する関数
void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& modelDirectory) {
	std::vector<VERTEX_3D> vertices;
	std::vector<UINT> indices;

	//頂点データを処理
	for (UINT i = 0; i < mesh->mNumVertices; i++) {
		VERTEX_3D vertex;

		//頂点座標
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
	}
}
