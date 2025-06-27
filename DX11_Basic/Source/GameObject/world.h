#pragma once

enum OBJECT_TYPE {
	TYPE_NONE = 0,
	TYPE_CAMERA,
	TYPE_3D,
	TYPE_2D,
	TYPE_MAX,
};

class GameObject;

class World {
public:

	bool Initialize();
	void Finalize();
	void Update(double deltaTime);
	void Draw() const;
	void CleanUp();

	GameObject* AddGameObject(GameObject* gameObject, OBJECT_TYPE type);

private:
	std::list<GameObject*> m_gameObjects[TYPE_MAX];
};

