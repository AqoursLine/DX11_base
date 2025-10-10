#pragma once

#include "main.h"

class GameObject {
public:
	//コンストラクタ
	GameObject() = default;

	//デストラクタ
	virtual ~GameObject() = default;

	bool InitializeBase()
	{
		if (!m_isInitialized) {
			m_isInitialized = Initialize();
		}
		return m_isInitialized;
	}
	virtual void Finalize() {}
	void UpdateBase(double deltaTime)
	{
		if (!m_isInitialized) {
			m_isInitialized = Initialize();
		}
		if (m_isActive) {
			Update(deltaTime);
		}
	}
	void DrawBase() {
		if (m_isVisible && m_isActive) {
			Draw();
		}
	}
	virtual void CleanUp() {}

	void SetActive(bool active) { m_isActive = active; }
	bool IsActive() const { return m_isActive; }

	void SetVisible(bool visible) { m_isVisible = visible; }
	bool IsVisible() const { return m_isVisible; }

	void SetDestroy(bool destroy) { m_isDestroy = destroy; }
	bool IsDestroy() {
		if (m_isDestroy) {
			Finalize();
			delete this;
			return true;
		} else {
			return false;
		}
	}

	GameObject* SetPosition(const Vector3& position) {
		m_position = position;
		return this;
	}
	GameObject* SetRotation(const Vector3& rotation) {
		m_rotation = rotation;
		return this;
	}
	GameObject* SetScale(const Vector3& scale) {
		m_scale = scale;
		return this;
	}

	Vector3 GetPosition() const { return m_position; }
	Vector3 GetRotation() const { return m_rotation; }
	Vector4 GetQuaternion() const { return m_quaternion; }
	Vector3 GetScale() const { return m_scale; }

	// 前方ベクトルを取得
	Vector3 GetForward() const {
		XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, rotMatrix.r[2]); // Z軸方向
		forward.Normalize();
		return forward;
	}
	//クォータニオン版
	Vector3 GetForwardQ() const {
		XMVECTOR quat = XMVectorSet(m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w);
		XMVECTOR forward = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), quat);
		Vector3 result;
		XMStoreFloat3((XMFLOAT3*)&result, forward);
		result.Normalize();
		return result;
	}

	// 右方向ベクトルを取得
	Vector3 GetRight() const {
		XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		Vector3 right;
		XMStoreFloat3((XMFLOAT3*)&right, rotMatrix.r[0]); // X軸方向
		right.Normalize();
		return right;
	}
	//クォータニオン版
	Vector3 GetRightQ() const {
		XMVECTOR quat = XMVectorSet(m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w);
		XMVECTOR right = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), quat);
		Vector3 result;
		XMStoreFloat3((XMFLOAT3*)&result, right);
		result.Normalize();
		return result;
	}

	// 上方向ベクトルを取得
	Vector3 GetUp() const {
		XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		Vector3 up;
		XMStoreFloat3((XMFLOAT3*)&up, rotMatrix.r[1]); // Y軸方向
		up.Normalize();
		return up;
	}
	//クォータニオン版
	Vector3 GetUpQ() const {
		XMVECTOR quat = XMVectorSet(m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w);
		XMVECTOR up = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), quat);
		Vector3 result;
		XMStoreFloat3((XMFLOAT3*)&result, up);
		result.Normalize();
		return result;
	}

protected:
	virtual bool Initialize() { return true; }
	virtual void Update(double deltaTime) {}
	virtual void Draw() const {}

	Vector3 m_position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector4 m_quaternion = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	Vector3 m_scale = Vector3(1.0f, 1.0f, 1.0f);
private:
	bool m_isActive = true;
	bool m_isVisible = true;
	bool m_isDestroy = false;

	bool m_isInitialized = false;
};


