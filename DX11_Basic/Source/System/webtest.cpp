#include "main.h"
#include "webtest.h"
#include <nlohmann/json.hpp>

GameWebSocketClient::GameWebSocketClient() : m_connected(false) {

}

GameWebSocketClient::~GameWebSocketClient() {
}

bool GameWebSocketClient::Connect(const std::string& url) {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cout << "WSAStartup failed: " << result << std::endl;
		return false;
	}
	// WebSocketクライアントの初期化
	m_client.setHandshakeTimeout(5); // タイムアウトを5秒に設定

	m_url = url;
	m_client.setUrl(m_url);

	//接続イベントの設定
	m_client.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
		if (msg->type == ix::WebSocketMessageType::Open) {
			m_connected = true;
			std::cout << "Connected to WebSocket server: " << m_url << std::endl;
		} else if (msg->type == ix::WebSocketMessageType::Close) {
			m_connected = false;
			std::cout << "Disconnected from WebSocket server." << std::endl;
		} else if (msg->type == ix::WebSocketMessageType::Error) {
			std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
		} else if (msg->type == ix::WebSocketMessageType::Message) {
			std::cout << "Received message: " << msg->str << std::endl;
		}
		});

	m_client.disableAutomaticReconnection(); // 自動再接続を無効化

	m_client.start();

	return true;
}

void GameWebSocketClient::SendMessage(const std::string& message) {
	if (m_connected) {
		// JSON形式のメッセージを作成
		nlohmann::json jsonMessage;
		jsonMessage["type"] = "message"; // メッセージのタイプ
		jsonMessage["message"] = message;
		jsonMessage["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
		jsonMessage["client_id"] = "client_123"; // クライアントIDなどの追加情報
		std::string jsonString = jsonMessage.dump();
		// メッセージを送信
		m_client.send(jsonString);
	}
}

void GameWebSocketClient::Disconnect() {
	m_client.stop();
}
