#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"

//Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef _DEBUG
	#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
	#pragma comment(lib, "assimp-vc143-mt.lib")
#endif // _DEBUG

//========================================================
//モデルの内部読み込み
//========================================================
MODEL* ModelRenderer::LoadModelInternal(const std::string& fileName) {
	//Assimpインポーター
	Assimp::Importer importer;

	//読み込みフラグ
	unsigned int flags =
		aiProcess_Triangulate |				//三角形化
		aiProcess_FlipUVs |					//UV反転
		aiProcess_CalcTangentSpace |		//接線空間計算
		aiProcess_JoinIdenticalVertices |	//頂点の結合
		aiProcess_GenSmoothNormals |		//スムーズ法線生成
		aiProcess_OptimizeMeshes |			//メッシュ最適化
		aiProcess_LimitBoneWeights			//ボーンウェイト制限
		;

	//シーンの読み込み
	const aiScene* scene = importer.ReadFile(fileName, flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::wstring errMsg = L"モデルの読み込みに失敗しました。\n";
		errMsg += std::wstring(importer.GetErrorString(), importer.GetErrorString() + strlen(importer.GetErrorString()));
		ErrorMessage(errMsg, E_FAIL);
		return nullptr;
	}

	//モデル構造体の作成
	MODEL* model = new MODEL();
	if (!BuildModelFromScene(scene, model)) {
		delete model;
		return nullptr;
	}

	return model;
}

//========================================================
//assimpシーンからモデルを構築
//========================================================
bool ModelRenderer::BuildModelFromScene(const aiScene* scene, MODEL* model) {
	//埋め込みテクスチャの読み込み
	if (!LoadEmbeddedTexture(scene, model)) {
		return false;
	}

	//マテリアルの処理
	if (!ProcessMaterial(scene, model)) {
		return false;
	}

	//メッシュの処理
	model->meshes.reserve(scene->mNumMeshes);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		if (!ProcessMesh(scene->mMeshes[i], model)) {
			return false;
		}
	}

	//ノードの処理
	model->rootNode = ProcessNode(scene->mRootNode, nullptr, model);
	if (!model->rootNode) {
		return false;
	}

	return true;
}

//========================================================
//埋め込みテクスチャの読み込み
//========================================================
bool ModelRenderer::LoadEmbeddedTexture(const aiScene* scene, MODEL* model) {
	//埋め込みテクスチャが無い場合は成功
	if (scene->mNumTextures == 0) {
		return true;
	}

	//埋め込みテクスチャを読み込み
	model->textures.reserve(scene->mNumTextures);
	for (unsigned int i = 0; i < scene->mNumTextures; i++) {
		aiTexture* texture = scene->mTextures[i];
		ComPtr<ID3D11ShaderResourceView> srv;

		//圧縮テクスチャの場合
		if (texture->mHeight == 0) {
			//DirectXTexでロード
			TexMetadata metadata;
			ScratchImage scratchImg;

			//フォーマットチェック
			std::string formatHint = texture->achFormatHint;
			HRESULT hr;

			if (formatHint == "png" || formatHint == "jpg" ||  formatHint == "jpeg") {
				//WICで読み込み
				hr = LoadFromWICMemory((const uint8_t*)texture->pcData, texture->mWidth, WIC_FLAGS_NONE, &metadata, scratchImg);
			} else if (formatHint == "dds") {
				//DDSで読み込み
				hr = LoadFromDDSMemory((const uint8_t*)texture->pcData, texture->mWidth, DDS_FLAGS_NONE, &metadata, scratchImg);
			} else {
				//デフォルトはWICで読み込み
				hr = LoadFromWICMemory((const uint8_t*)texture->pcData, texture->mWidth, WIC_FLAGS_NONE, &metadata, scratchImg);
			}

			if (FAILED(hr)) {
				std::wstring errMsg = L"埋め込みテクスチャの読み込みに失敗しました。\n";
				errMsg += L"フォーマット：" + std::wstring(formatHint.begin(), formatHint.end()) + L"\n";
				ErrorMessage(errMsg, hr);

				//ダミーテクスチャを設定して続行
				model->textures.push_back(nullptr);
				continue;
			}

			//SRVの作成
			hr = CreateShaderResourceView(RENDERER.GetDevice(), scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, &srv);

			if (FAILED(hr)) {
				std::wstring errMsg = L"埋め込みテクスチャのSRV作成に失敗しました。\n";
				errMsg += L"フォーマット：" + std::wstring(formatHint.begin(), formatHint.end()) + L"\n";
				ErrorMessage(errMsg, hr);
				//ダミーテクスチャを設定して続行
				model->textures.push_back(nullptr);
				continue;
			}
		} else {
			//非圧縮テクスチャの場合
			//テクスチャ記述子を作成
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = texture->mWidth;
			desc.Height = texture->mHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			//テクスチャデータの準備
			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = texture->pcData;
			initData.SysMemPitch = texture->mWidth * 4; //RGBA8

			//テクスチャの作成
			ComPtr<ID3D11Texture2D> tex;
			HRESULT hr = RENDERER.GetDevice()->CreateTexture2D(&desc, &initData, &tex);

			if (FAILED(hr)) {
				ErrorMessage(L"埋め込みテクスチャの作成に失敗しました。", hr);
				//ダミーテクスチャを設定して続行
				model->textures.push_back(nullptr);
				continue;
			}

			//SRVの作成
			hr = RENDERER.GetDevice()->CreateShaderResourceView(tex.Get(), nullptr, &srv);
			if (FAILED(hr)) {
				ErrorMessage(L"埋め込みテクスチャのSRV作成に失敗しました。", hr);
				//ダミーテクスチャを設定して続行
				model->textures.push_back(nullptr);
				continue;
			}
		}

		//テクスチャ配列に追加
		model->textures.push_back(srv);
	}

	return true;
}

//========================================================
//マテリアルの処理
//========================================================
bool ModelRenderer::ProcessMaterial(const aiScene* scene, MODEL* model) {
	model->materials.reserve(scene->mNumMaterials);

	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* aiMat = scene->mMaterials[i];
		MODEL_MATERIAL material;

		//マテリアル名
		aiString name;
		if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
			material.name = name.C_Str();
		} else {
			material.name = "Material_" + std::to_string(i);
		}

		//基本色
		aiColor4D color;
		material.diffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
			material.diffuseColor = Vector4(color.r, color.g, color.b, color.a);
		}
		material.specularColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
			material.specularColor = Vector4(color.r, color.g, color.b, color.a);
		}
		material.ambientColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
			material.ambientColor = Vector4(color.r, color.g, color.b, color.a);
		}
		material.emissiveColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
			material.emissiveColor = Vector4(color.r, color.g, color.b, color.a);
		}

		//物理パラメータ
		material.shininess = 32.0f;
		aiMat->Get(AI_MATKEY_SHININESS, material.shininess);

		material.opacity = 1.0f;
		aiMat->Get(AI_MATKEY_OPACITY, material.opacity);

		material.metallic = 0.0f;
		aiMat->Get(AI_MATKEY_METALLIC_FACTOR, material.metallic);

		material.roughness = 1.0f;
		aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, material.roughness);

		//フラグを取得
		int twoSided = 0;
		aiMat->Get(AI_MATKEY_TWOSIDED, twoSided);
		material.isTwoSided = (twoSided != 0);

		material.shadingModel = 0;
		aiMat->Get(AI_MATKEY_SHADING_MODEL, material.shadingModel);

		//テクスチャ情報を取得
		aiTextureType textureTypes[] = {
			aiTextureType_DIFFUSE,
			aiTextureType_SPECULAR,
			aiTextureType_NORMALS,
			aiTextureType_EMISSIVE,
		};

		for (aiTextureType type : textureTypes) {
			unsigned int count = aiMat->GetTextureCount(type);

			for (unsigned int j = 0; j < count; j++) {
				aiString path;
				unsigned int uvIndex = 0;

				if (aiMat->GetTexture(type, j, &path, nullptr, &uvIndex, nullptr, nullptr, nullptr) == AI_SUCCESS)  {
					std::string texPath = path.C_Str();

					//埋め込みテクスチャの場合はインデックスを取得
					if (texPath[0] == '*') {
						int embedIndex = std::stoi(texPath.c_str() + 1);

						//インデックスが範囲内かチェック
						if (embedIndex >= 0 && embedIndex < (int)model->textures.size()) {
							MODEL_MATERIAL_TEXTURE matTex;
							matTex.textureIndex = embedIndex;
							matTex.TextureType = type;
							matTex.uvChannel = uvIndex;
							material.textures.push_back(matTex);
						}
					}
				}
			}
		}

		//マテリアルを追加
		model->materials.push_back(material);
	}

	return true;
}

//========================================================
//メッシュの処理
//========================================================
bool ModelRenderer::ProcessMesh(const aiMesh* aiMesh, MODEL* model) {
	MODEL_MESH mesh;

	//頂点データを処理
	mesh.vertices.reserve(aiMesh->mNumVertices);

	for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
		MODEL_VERTEX vertex;

		//位置
		vertex.position = Vector4(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z, 1.0f);

		//法線
		if (aiMesh->HasNormals()) {
			vertex.normal = Vector4(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z, 0.0f);
		} else {
			vertex.normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		}

		//uv座標
		if (aiMesh->mTextureCoords[0]) {
			vertex.texcoord = Vector2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
		} else {
			vertex.texcoord = Vector2(0.0f, 0.0f);
		}

		//接線
		if (aiMesh->HasTangentsAndBitangents()) {
			vertex.tangent = Vector4(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z, 0.0f);
		} else {
			vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		}

		//頂点カラー
		if (aiMesh->mColors[0]) {
			vertex.color = Vector4(aiMesh->mColors[0][i].r, aiMesh->mColors[0][i].g, aiMesh->mColors[0][i].b, aiMesh->mColors[0][i].a);
		} else {
			vertex.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		mesh.vertices.push_back(vertex);
	}

	//インデックスデータを処理
	mesh.indices.reserve(aiMesh->mNumFaces * 3);

	for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
		aiFace face = aiMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			mesh.indices.push_back(face.mIndices[j]);
		}
	}

	//マテリアルインデックス
	mesh.materialIndex = aiMesh->mMaterialIndex;

	//GPUバッファの作成
	if (!CreateBuffers(mesh)) {
		return false;
	}

	//メッシュを追加
	model->meshes.push_back(mesh);

	return true;
}

//========================================================
//ノードの処理
//========================================================
std::unique_ptr<MODEL_NODE> ModelRenderer::ProcessNode(const aiNode* aiNode, MODEL_NODE* parent, MODEL* model) {
	//新しいノードを作成
	auto node = std::make_unique<MODEL_NODE>();

	//ノード名
	node->name = aiNode->mName.C_Str();

	//親ノードを設定
	node->parent = parent;

	//ローカルトランスフォーム
	XMMATRIX mat;
	aiMatrix4x4 aiMat = aiNode->mTransformation;
	mat.r[0] = XMVectorSet(aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1);
	mat.r[1] = XMVectorSet(aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2);
	mat.r[2] = XMVectorSet(aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3);
	mat.r[3] = XMVectorSet(aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4);
	node->localTransform = mat;

	//このノードが持つメッシュのインデックスを保存
	node->meshIndices.reserve(aiNode->mNumMeshes);
	for (unsigned int i = 0; i < aiNode->mNumMeshes; i++) {
		node->meshIndices.push_back(aiNode->mMeshes[i]);
	}

	//ノードマップに登録
	if (!node->name.empty()) {
		model->nodeMap[node->name] = node.get();
	}

	//子ノードを再帰的に処理
	node->children.reserve(aiNode->mNumChildren);
	for (unsigned int i = 0; i < aiNode->mNumChildren; i++) {
		node->children.push_back(ProcessNode(aiNode->mChildren[i], node.get(), model));
	}

	return node;
}

bool ModelRenderer::CreateBuffers(MODEL_MESH& mesh) {
	HRESULT hr;

	//頂点バッファの作成
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = static_cast<UINT>(sizeof(MODEL_VERTEX) * mesh.vertices.size());
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = mesh.vertices.data();

	hr = RENDERER.GetDevice()->CreateBuffer(&vbDesc, &vbData, &mesh.vertexBuffer);

	if (FAILED(hr)) {
		ErrorMessage(L"頂点バッファの作成に失敗しました。", hr);
		return false;
	}

	//インデックスバッファの作成
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * mesh.indices.size());
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = mesh.indices.data();
	hr = RENDERER.GetDevice()->CreateBuffer(&ibDesc, &ibData, &mesh.indexBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"インデックスバッファの作成に失敗しました。", hr);
		return false;
	}

	mesh.indexCount = static_cast<unsigned int>(mesh.indices.size());

	return true;
}
