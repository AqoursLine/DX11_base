#include "main.h"
#include "system.h"
#include "manager.h"
#include "timer.h"
#include "Component/model.h"
#include "webtest.h"
#include "physics.h"

#ifdef _DEBUG
#include "input.h"
#endif // _DEBUG

// シングルトンインスタンスの初期化
System* System::s_instance = nullptr;

//システム初期化
bool System::Initialize() {
	//物理エンジンの初期化
	m_physics = new Physics();
	if (!m_physics->Initialize()) {
		ErrorMessage(L"物理エンジンの初期化に失敗しました", E_FAIL);
		return false;
	}

	//マネージャークラス
	m_manager = new Manager();
	bool isInitialized = m_manager->Initialize();

	if (!isInitialized) {
		ErrorMessage(L"マネージャークラスの初期化に失敗しました", E_FAIL);
		return false;
	}

	//タイマー初期化
	m_timer = new Timer();
	m_timer->Reset();
	m_timer->Start();

	m_webSocketClient = new GameWebSocketClient();
	//WebSocketクライアントの接続
	if (!m_webSocketClient->Connect("ws://localhost:9002")) {
		std::cerr << "WebSocket client connection failed." << std::endl;
	} else {
		std::cout << "WebSocket client connected successfully." << std::endl;
	}

	return true;
}

//システム終了
void System::Finalize() {
	//マネージャークラスの終了
	if (m_manager) {
		m_manager->Finalize();
		delete m_manager;
		m_manager = nullptr;
	}

	//モデルのキャッシュをクリア
	Model::ClearCache();

	//物理エンジンの終了
	if (m_physics) {
		m_physics->Finalize();
		delete m_physics;
		m_physics = nullptr;
	}

	//タイマー終了
	if (m_timer) {
		m_timer->Stop();
		m_timer->Reset();
		delete m_timer;
		m_timer = nullptr;
	}

	//WebSocketクライアントの終了
	if (m_webSocketClient) {
		m_webSocketClient->Disconnect();
		delete m_webSocketClient;
		m_webSocketClient = nullptr;
	}
}

bool System::Excute() {
	m_timer->Tick();

	m_physics->Update(m_timer->GetDeltaTime());

	m_manager->Update(m_timer->GetDeltaTime());
	m_manager->Draw();
	if (m_manager->CleanUp()) {
		return true;
	}

#ifdef _DEBUG
	if (Input::GetKeyTrigger(KK_LEFTCONTROL)) {
		if (m_webSocketClient->IsConnected()) {
			std::string message = "Hello from the client!";
			m_webSocketClient->SendMessage(message);
			std::cout << "Message sent: " << message << std::endl;
		}
	}
#endif
	return false;
}
