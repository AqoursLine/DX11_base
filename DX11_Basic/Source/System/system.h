#pragma once

class Timer;
class Manager;

class System {
public:
	System() = default;
	~System() = default;

	bool Initialize();
	void Finalize();
	bool Excute();

private:
	Timer* m_timer = nullptr;
	Manager* m_manager = nullptr;
	class GameWebSocketClient* m_webSocketClient = nullptr; // WebSocketクライアントのポインタ
	bool m_isConnected = false;
};
