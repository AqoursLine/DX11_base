#include "main.h"
#include "system.h"
#include "manager.h"
#include "timer.h"
#include "Component/model.h"
#include "webtest.h"

#ifdef _DEBUG
#include "input.h"
#endif // _DEBUG


//システム初期化
bool System::Initialize() {
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
		m_isConnected = false;
		std::cerr << "WebSocket client connection failed." << std::endl;
	} else {
		m_isConnected = true;
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
