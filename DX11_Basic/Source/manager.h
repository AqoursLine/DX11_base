#pragma once

class Sprite;
class Camera;
class Field;

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

	Sprite* m_sprite = nullptr;
	Camera* m_camera = nullptr;
	Field* m_field = nullptr;

};
