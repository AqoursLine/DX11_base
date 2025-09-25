#pragma once

#include <list>
#include <vector>

enum OBJECT_TYPE {
	TYPE_NONE = 0,
	TYPE_CAMERA,
	TYPE_3D,
	TYPE_2D,
	TYPE_MAX,
};

class GameObject;

class Scene {
public:

	virtual bool Initialize();
	virtual void Finalize();
	virtual void Update(double deltaTime);
	virtual void Draw();
	virtual void CleanUp();

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
private:
	std::list<GameObject*> m_gameObjects[TYPE_MAX];
};

