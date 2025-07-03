#pragma once

#include "main.h"

class World;

class GameObject {
public:
	//コンストラクタ
	GameObject() = default;

	//デストラクタ
	~GameObject() = default;

	virtual bool Initialize() { return true; }
	virtual void Finalize() {}
	virtual void Update(double deltaTime) {}
	virtual void Draw() const {}
	virtual void CleanUp() {}

	void SetWorld(World* world) { m_world = world; }

	void SetActive(bool active) { isActive = active; }
	bool IsActive() const { return isActive; }

	void SetVisible(bool visible) { isVisible = visible; }
	bool IsVisible() const { return isVisible; }

	void SetDestroy(bool destroy) { isDestroy = destroy; }
	bool IsDestroy() const { return isDestroy; }

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
	Vector3 GetScale() const { return m_scale; }

protected:
	Vector3 m_position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_scale = Vector3(1.0f, 1.0f, 1.0f);

	World* m_world = nullptr;
private:
	bool isActive = true;
	bool isVisible = true;
	bool isDestroy = false;
};


