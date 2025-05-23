#pragma once

#include "main.h"

enum OBJECT_TYPE {
	TYPE_NONE = 0,
	TYPE_CAMERA,
	TYPE_3D,
	TYPE_2D,
};

class World;

class GameObject {
public:
	//コンストラクタ
	GameObject() = delete;
	GameObject(OBJECT_TYPE type) : m_type(type) {}

	//デストラクタ
	~GameObject() = default;

	virtual bool Initialize() { return true; }
	virtual void Finalize() {}
	virtual void Update(double deltaTime) {}
	virtual void Draw() const {}
	virtual void CleanUp() {}

	void SetWorld(World* world) { m_world = world; }

	const OBJECT_TYPE GetType() const { return m_type; }

	void SetActive(bool active) { isActive = active; }
	bool IsActive() const { return isActive; }

	void SetVisible(bool visible) { isVisible = visible; }
	bool IsVisible() const { return isVisible; }

	void SetDestroy(bool destroy) { isDestroy = destroy; }
	bool IsDestroy() const { return isDestroy; }

	Vector3 GetPosition() const { return m_position; }
	Vector3 GetRotation() const { return m_rotation; }
	Vector3 GetScale() const { return m_scale; }

protected:
	OBJECT_TYPE m_type = TYPE_NONE;

	Vector3 m_position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_scale = Vector3(1.0f, 1.0f, 1.0f);

	World* m_world = nullptr;
private:
	bool isActive = true;
	bool isVisible = true;
	bool isDestroy = false;
};


