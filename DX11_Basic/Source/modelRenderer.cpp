#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"

//静的メンバーの初期化
std::unordered_map<std::string, std::unique_ptr<MODEL>> ModelRenderer::m_modelCache;

//========================================================
//モデルノードの変換行列計算
//========================================================
XMMATRIX MODEL_NODE::GetWorldTransform() const {
	if (parent) {
		return localTransform * parent->GetWorldTransform();
	}
	else {
		return localTransform;
	}
}

//========================================================
//モデルの読み込み
//========================================================
bool ModelRenderer::Load(const std::string& fileName) {
	//モデルキャッシュを確認
	if (m_modelCache.count(fileName)) {
		m_model = m_modelCache[fileName].get();
		return true; //キャッシュから取得成功
	}
	//モデルの読み込み
	m_model = LoadModelInternal(fileName);
	if (!m_model) {
		return false;
	}
	//キャッシュに追加
	m_modelCache[fileName] = std::unique_ptr<MODEL>(m_model);
	return true;
}

//========================================================
//描画(オイラー角)
//========================================================
void ModelRenderer::Draw(const Vector3& position, const Vector3& rotation, const Vector3& scale) {
	if (!m_model) {
		return;
	}

	//オイラー角からワールド行列を計算
	XMMATRIX world =
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		XMMatrixTranslation(position.x, position.y, position.z);

	DrawInternal(world);
}

//========================================================
//描画(クォータニオン)
//========================================================
void ModelRenderer::Draw(const Vector3& position, const Vector4& rotation, const Vector3& scale) {
	if (!m_model) {
		return;
	}

	//クォータニオンから回転行列を計算
	XMMATRIX rot = XMMatrixRotationQuaternion(
		XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w)
	);

	//ワールド行列を計算
	XMMATRIX world =
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		rot *
		XMMatrixTranslation(position.x, position.y, position.z);

	DrawInternal(world);
}

void ModelRenderer::DrawInternal(const XMMATRIX& world) {
	//プリミティブトポロジー設定
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ルートノードから描画
	if (m_model->rootNode) {
		DrawNode(m_model->rootNode.get(), world);
	}
}

void ModelRenderer::DrawNode(MODEL_NODE* node, const XMMATRIX& parentTransform) {
	if (!node) {
		return;
	}

	//ノードのワールド行列を計算
	XMMATRIX world = node->localTransform * parentTransform;

	//ノードに含まれるメッシュを描画
	for (unsigned int meshIndex : node->meshIndices) {
		if (meshIndex < m_model->meshes.size()) {
			DrawMesh(m_model->meshes[meshIndex], world);
		}
	}

	//子ノードを再帰的に描画
	for (auto& child : node->children) {
		DrawNode(child.get(), world);
	}
}

void ModelRenderer::DrawMesh(const MODEL_MESH& mesh, const XMMATRIX& world) {
	//ワールド行列をシェーダーに設定
	RENDERER.SetWorldMatrix(world);

	//マテリアル設定
	if (mesh.materialIndex < m_model->materials.size()) {
		SetMaterial(m_model->materials[mesh.materialIndex]);
	}

	//頂点バッファ設定
	UINT stride = sizeof(MODEL_VERTEX);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);

	//インデックスバッファ設定
	RENDERER.GetDeviceContext()->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//描画
	RENDERER.GetDeviceContext()->DrawIndexed(mesh.indexCount, 0, 0);
}

void ModelRenderer::SetMaterial(const MODEL_MATERIAL& material) const {
	//マテリアル設定
	MATERIAL rendererMaterial = {};

	//基本色
	rendererMaterial.diffuse = XMFLOAT4(
		material.diffuseColor.x,
		material.diffuseColor.y,
		material.diffuseColor.z,
		material.diffuseColor.w
	);
	rendererMaterial.specular = XMFLOAT4(
		material.specularColor.x,
		material.specularColor.y,
		material.specularColor.z,
		material.shininess //w成分に光沢度を格納
	);
	rendererMaterial.ambient = XMFLOAT4(
		material.ambientColor.x,
		material.ambientColor.y,
		material.ambientColor.z,
		material.ambientColor.w
	);
	rendererMaterial.emission = XMFLOAT4(
		material.emissiveColor.x,
		material.emissiveColor.y,
		material.emissiveColor.z,
		material.emissiveColor.w
	);

	//物理パラメータ
	rendererMaterial.shininess = material.shininess;

	//テクスチャ設定
	rendererMaterial.textureEnable = !material.textures.empty();

	//マテリアルをレンダラーに設定
	RENDERER.SetMaterial(rendererMaterial);

	//テクスチャ設定
	for (const auto& texInfo : material.textures) {
		if (texInfo.textureIndex >= 0 && texInfo.textureIndex < static_cast<int>(m_model->textures.size())) {

			//テクスチャタイプに応じてスロットを変更
			UINT slot = 0; //デフォルトはスロット0
			switch (texInfo.TextureType) {
				case 1: slot = 0; break; //diffuse
				case 2: slot = 1; break; //specular
				case 6: slot = 2; break; //normal
				case 4: slot = 3; break; //emissive
				default: slot = 0; break; //その他はスロット0
			}

			//テクスチャをシェーダーに設定
			RENDERER.GetDeviceContext()->PSSetShaderResources(slot, 1, m_model->textures[texInfo.textureIndex].GetAddressOf());
		}
	}

}

//========================================================
//モデルの解放
//========================================================
void ModelRenderer::ReleaseAll() {
	m_modelCache.clear();
}
