#pragma once

class Scene;
class Transition;

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
	bool IsFinished() const { return m_isFinished; }

	Scene* GetScene() const { return m_scene; }

	void SetScene(Scene* scene, Transition* transition);
private:
	//終了したか
	bool m_isFinished = false;

	Scene* m_scene = nullptr;
	Scene* m_nextScene = nullptr;

	Transition* m_transition = nullptr;

	bool m_isChangingScene = false;

	class ImguiSystem* m_imguiSystem = nullptr;
};
