#pragma once

class Sprite;

class Manager {
public:
	Manager();
	~Manager();

	bool Initialize();
	void Finalize();

	bool Update(double dt);
	void Draw() const;

	void CleanUp();

	static void SetFinish(bool isFinish) { m_isFinished = isFinish; }
private:
	static bool m_isFinished;

	Sprite* m_sprite = nullptr;
};
