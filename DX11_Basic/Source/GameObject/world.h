#pragma once

class GameObject;

class World {
public:

	bool Initialize();
	void Finalize();
	void Update(double deltaTime);
	void Draw() const;
	void CleanUp();

	void AddGameObject(GameObject* gameObject);

private:
	std::list<GameObject*> m_gameObjects;
	std::vector<GameObject*> m_gameObjectsAddList;

	void SetGameObject();
};

