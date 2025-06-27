#pragma once

class World;

class Manager {
public:
	Manager();
	~Manager();

	bool Initialize();
	void Finalize();

	void Update(double dt);
	void Draw() const;

	bool CleanUp();

	static void SetFinish(bool isFinish) { m_isFinished = isFinish; }
private:
	//終了したか
	static bool m_isFinished;

	World* m_world;
};
