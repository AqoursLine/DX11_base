#include "main.h"
#include "system.h"
#include "manager.h"
#include "timer.h"
#include "model.h"
#include "webClient.h"

#ifdef _DEBUG
#include "input.h"
#endif // _DEBUG

// シングルトンインスタンスの初期化
System* System::s_instance = nullptr;

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

	//WebClient初期化
	m_webClient = new WebClient();

	//===コールバック関数設定====
	//接続成功時
	m_webClient->SetOnConnected([]() {
		std::cout << "WebSocket connected." << std::endl;
		});
	//切断時
	m_webClient->SetOnDisconnected([]() {
		std::cout << "WebSocket disconnected." << std::endl;
		});
	//メッセージ受信時の処理
	m_webClient->SetOnMessage([](const json& message) {
		std::cout << "Received message: " << message.dump() << std::endl;
		});
	//エラー発生時
	m_webClient->SetOnError([](const std::string& error) {
		std::cerr << "WebSocket error: " << error << std::endl;
		});

	//=======================

	//サーバーに接続
	if (!m_webClient->Connect("ws://localhost:9002")) {
		ErrorMessage(L"WebSocketサーバーへの接続に失敗しました", E_FAIL);
		return false;
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

	//WebClient終了
	if (m_webClient) {
		m_webClient->Disconnect();
		delete m_webClient;
		m_webClient = nullptr;
	}
}

bool System::Excute() {
	m_timer->Tick();

	//WebClientのメッセージ処理
	m_webClient->ProcessMessages();

	//マネージャークラス更新
	m_manager->Update(m_timer->GetDeltaTime());
	m_manager->Draw();
	if (m_manager->CleanUp()) {
		return true;
	}

	//

	return false;
}
