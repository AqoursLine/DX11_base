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

	void SetFinish(bool isFinish) { m_isFinished = isFinish; }

	Scene* GetScene() const { return m_scene; }

	void SetScene(Scene* scene);
private:
	//終了したか
	bool m_isFinished = false;

	Scene* m_scene = nullptr;
	Scene* m_nextScene = nullptr;

	class ImguiSystem* m_imguiSystem = nullptr;
};
