#pragma once

class Timer;
class Manager;
class PhysicsWorld;

class System {
public:
	~System() = default;

	bool Initialize();
	void Finalize();
	bool Excute();

	Manager* GetManager() const { return m_manager; }
	PhysicsWorld* GetPhysicsWorld() const { return m_physicsWorld; }

private:
	static System* s_instance;
	System() = default;


	Timer* m_timer = nullptr;
	Manager* m_manager = nullptr;
	class GameWebSocketClient* m_webSocketClient = nullptr; // WebSocketクライアントのポインタ
	PhysicsWorld* m_physicsWorld = nullptr;

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
