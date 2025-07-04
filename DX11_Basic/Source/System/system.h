#pragma once

class Timer;
class Manager;
class GameWebSocketClient;
class Physics;

class System {
public:
	~System() = default;

	bool Initialize();
	void Finalize();
	bool Excute();

	Manager* GetManager() const { return m_manager; }
	GameWebSocketClient* GetWebSocketClient() const { return m_webSocketClient; }
	Physics* GetPhysics() const { return m_physics; }

private:
	static System* s_instance;
	System() = default;


	Timer* m_timer = nullptr;
	Manager* m_manager = nullptr;
	GameWebSocketClient* m_webSocketClient = nullptr; // WebSocketクライアントのポインタ
	Physics* m_physics = nullptr; // Physicsクラスのポインタ
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
