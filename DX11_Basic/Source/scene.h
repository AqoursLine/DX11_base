#pragma once

#include <list>
#include <vector>

enum OBJECT_TYPE {
	TYPE_NONE = 0,
	TYPE_CAMERA,
	TYPE_LIGHT,
	TYPE_OPAQUE,
	TYPE_CUTOUT,
	TYPE_TRANSPARENT,
	TYPE_BEFORE_PROCESS_UI,
	TYPE_POST_PROCESS,
	TYPE_AFTER_PROCESS_UI,
	
	TYPE_MAX,
};

class GameObject;
class Camera;

class Scene {
public:

	virtual bool Initialize();
	virtual void Finalize();
	virtual void Update(double deltaTime);
	virtual void Draw();
	virtual void CleanUp();

	virtual Vector2 GetBoundsMin() const { return m_boundsMin; }
	virtual Vector2 GetBoundsMax() const { return m_boundsMax; }

	GameObject* AddGameObject(GameObject* gameObject, OBJECT_TYPE type);

	template <typename T>
	T* GetGameObject() {
		for (auto& objects : m_gameObjects) {
			for (auto& gameObject : objects) {
				T* obj = dynamic_cast<T*>(gameObject);
				if (obj) {
					return obj;
				}
			}
		}
		return nullptr;
	}

	template <typename T>
	std::vector<T*> GetGameObjects() {
		std::vector<T*> objects;
		for (auto& objectsList : m_gameObjects) {
			for (auto& gameObject : objectsList) {
				T* obj = dynamic_cast<T*>(gameObject);
				if (obj) {
					objects.push_back(obj);
				}
			}
		}
		return objects;
	}

protected:
private:
	std::list<GameObject*> m_gameObjects[TYPE_MAX];

	//描画関数
	void DrawLights() const;
	void DrawOpaque(Camera* camera) const;
	void DrawCutout(Camera* camera) const;
	void DrawTransparent(Camera* camera) const;
	void DrawBeforeEffect() const;
	void DrawPostProcess();
	void DrawAfterEffect() const;

	//ポストプロセス用
	int m_renderTargetIndex = 0;

	Vector2 m_boundsMin = { -1000.0f, -1000.0f };
	Vector2 m_boundsMax = { 1000.0f, 1000.0f };

};

