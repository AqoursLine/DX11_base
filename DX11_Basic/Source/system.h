#pragma once

class Timer;
class Manager;

class System {
public:
	~System() = default;

	bool Initialize();
	void Finalize();
	bool Excute();

	Manager* GetManager() const { return m_manager; }

private:
	static System* s_instance;
	System() = default;


	Timer* m_timer = nullptr;
	Manager* m_manager = nullptr;

public:
	static System* CreateInstance() {
		DestroyInstance();

		s_instance = new System();
		return s_instance;
	}
	static System& GetInstance() {
		return *s_instance;
	}
	static void DestroyInstance() {
		if (s_instance) {
			delete s_instance;
			s_instance = nullptr;
		}
	}
};

#define SYSTEM System::GetInstance()
