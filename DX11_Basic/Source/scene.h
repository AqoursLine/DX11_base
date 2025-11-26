#pragma once

#include <list>
#include <vector>
#include <atomic>
#include <future>

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
	virtual ~Scene() { if (m_future.valid()) m_future.wait(); }

	bool InitializeBase();
	void FinalizeBase();
	void UpdateBase(double deltaTime);
	void DrawBase();
	void CleanUpBase();

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

	template <typename T>
	std::vector<T*> GetGameObjectsOfType(OBJECT_TYPE type) {
		std::vector<T*> objects;
		for (auto& gameObject : m_gameObjects[type]) {
			T* obj = dynamic_cast<T*>(gameObject);
			if (obj) {
				objects.push_back(obj);
			}
		}
		return objects;
	}

	bool IsInitialized() const { return m_isInitialized.load(std::memory_order_acquire); }
	bool IsInitializedBase() const { return m_isInitializedBase; }

protected:
	virtual bool Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update(double deltaTime) = 0;
	virtual void Draw() = 0;
	virtual void CleanUp() = 0;
private:
	//ゲームオブジェクトリスト
	std::list<GameObject*> m_gameObjects[TYPE_MAX];

	//オブジェクト初期化関数
	bool ObjectInitialize();
	void ObjectFinalize();
	void ObjectUpdate(double deltaTime);
	void ObjectDestroy();
	void ObjectDraw();
	
	//初期化フラグ
	std::atomic<bool> m_isInitialized { false };
	bool m_isInitializedBase = false;

	//セーフティ
	std::future<void> m_future;

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

