#include "main.h"
#include "renderer.h"
#include "shaders.h"
#include "gpuParticleSystem.h"

/// <summary>
/// パーティクル発生
/// </summary>
/// <param name="count">発生数</param>
void GPUParticleSystem::Emit(int count) {
	EmitGPU(count);
}

/// <summary>
/// ワンショット発生
/// </summary>
/// <param name="position">発生位置</param>
/// <param name="count">発生数</param>
void GPUParticleSystem::EmitOneShot(const Vector3& position, int count) {
	// エミッター位置を一時的に変更
	Vector3 originalPosition = m_settings.position;
	m_settings.position = position;
	// ワンショット発生
	EmitGPU(count > 0 ? count : m_settings.oneShotCount);
	// エミッター位置を元に戻す
	m_settings.position = originalPosition;
}

/// <summary>
/// 初期化
/// </summary>
/// <returns>成功フラグ</returns>
bool GPUParticleSystem::Initialize() {
	// 乱数初期化

	// バッファ作成
	if (!CreateBuffers()) {
		return false;
	}

	// シェーダ読み込み
	if (!LoadShaders()) {
		return false;
	}

	// ブレンドステート作成
	if (!CreateState()) {
		return false;
	}

	return true;
}

/// <summary>
/// 終了
/// </summary>
void GPUParticleSystem::Finalize() {
	// シェーダー解放
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
	if (m_updateComputeShader) {
		delete m_updateComputeShader;
		m_updateComputeShader = nullptr;
	}
	if (m_emitComputeShader) {
		delete m_emitComputeShader;
		m_emitComputeShader = nullptr;
	}
	if (m_compactComputeShader) {
		delete m_compactComputeShader;
		m_compactComputeShader = nullptr;
	}

}

/// <summary>
/// 更新
/// </summary>
/// <param name="deltaTime"></param>
void GPUParticleSystem::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	if (m_isPaused) {
		return;
	}

	// パーティクル更新
	if (m_isPlaying && !m_settings.oneShot) {
		m_emitTimer += dt;
		float emitInterval = 1.0f / m_settings.emitRate;

		while (m_emitTimer >= emitInterval) {
			EmitGPU(1);
			m_emitTimer -= emitInterval;
		}
	}

	// パーティクル更新
	UpdateParticlesGPU(dt);

	// IndirectDrawモードの場合、アクティブパーティクルを収集
	if (m_drawMode == ParticleDrawMode::INDIRECT_DRAW) {
		CompactParticlesGPU();
	}
}

/// <summary>
/// 描画
/// </summary>
void GPUParticleSystem::Draw() {
	if (!m_textureSRV) {
		return;
	}

	auto context = RENDERER.GetDeviceContext();

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// ブレンドステート設定
	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);

	// 頂点バッファ設定
	UINT stride = sizeof(BillboardVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画モード分岐
	if (m_drawMode == ParticleDrawMode::CULL_DISTANCE) {
		// CullDistanceモード: 全パーティクル描画(非アクティブはシェーダー内で破棄)
		context->VSSetShaderResources(0, 1, m_particleBufferSRV.GetAddressOf());
		context->PSSetShaderResources(1, 1, &m_textureSRV);

		// インデックスドインスタンスド描画
		context->DrawIndexedInstanced(6, static_cast<UINT>(m_settings.maxParticles), 0, 0, 0);

		ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
		context->VSSetShaderResources(0, 1, nullSRV);
		context->PSSetShaderResources(1, 1, nullSRV);
	} else {
		// IndirectDrawモード: アクティブパーティクルのみ描画
		ID3D11ShaderResourceView* srvs[2] = { m_particleBufferSRV.Get(), m_activeIndicesSRV.Get() };
		context->VSSetShaderResources(0, 2, srvs);
		context->PSSetShaderResources(1, 1, &m_textureSRV);

		// 間接描画
		context->DrawIndexedInstancedIndirect(m_drawArgsBuffer.Get(), 0);

		ID3D11ShaderResourceView* nullSRV[3] = { nullptr, nullptr, nullptr };
		context->VSSetShaderResources(0, 2, nullSRV);
		context->PSSetShaderResources(1, 1, nullSRV);
	}

	// ブレンドステート解除
	RENDERER.SetATCEnable(false);
}

/// <summary>
/// パーティクル更新(GPU)
/// </summary>
/// <param name="deltaTime"></param>
void GPUParticleSystem::UpdateParticlesGPU(float deltaTime) {
	auto context = RENDERER.GetDeviceContext();

	// 更新用パラメータ設定
	UpdateParams params;
	params.deltaTime = deltaTime;
	params.gravity = m_settings.gravity;
	params.startSize = m_settings.startSize;
	params.endSize = m_settings.endSize;
	params.startColor = XMFLOAT4(m_settings.startColor.x, m_settings.startColor.y, m_settings.startColor.z, m_settings.startColor.w);
	params.endColor = XMFLOAT4(m_settings.endColor.x, m_settings.endColor.y, m_settings.endColor.z, m_settings.endColor.w);

	// パラメータバッファに転送
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_updateParamsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &params, sizeof(UpdateParams));
	context->Unmap(m_updateParamsBuffer.Get(), 0);

	// コンピュートシェーダー設定
	m_updateComputeShader->Set();

	// パラメータバッファ設定
	context->CSSetConstantBuffers(3, 1, m_updateParamsBuffer.GetAddressOf());

	// UAV設定
	context->CSSetUnorderedAccessViews(0, 1, m_particleBufferUAV.GetAddressOf(), nullptr);

	// ディスパッチ
	UINT threadGroupCount = (m_settings.maxParticles + 255) / 256;
	context->Dispatch(threadGroupCount, 1, 1);

	// リソース解除
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ID3D11Buffer* nullBuffer = nullptr;
	context->CSSetConstantBuffers(3, 1, &nullBuffer);
}

/// <summary>
/// パーティクル発生(GPU)
/// </summary>
/// <param name="count">発生数</param>
void GPUParticleSystem::EmitGPU(UINT count) {
	auto context = RENDERER.GetDeviceContext();

	// 乱数シード更新
}
