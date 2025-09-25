#pragma once

class Scene;

class Manager {
public:
	Manager();
	~Manager();

	bool Initialize();
	void Finalize();

	void Update(double dt);
	void Draw();
	bool CleanUp();

	static void SetFinish(bool isFinish) { m_isFinished = isFinish; }

	Scene* GetScene() const { return m_scene; }

	void SetScene(Scene* scene);
private:
	//終了したか
	static bool m_isFinished;

	Scene* m_scene;
	Scene* m_nextScene = nullptr;
};
