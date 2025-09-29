#include "webClient.h"
#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>

#ifndef _MSC_VER
#error "This project requires visual studio compiler."
#endif // !_MSC_VER


// ライブラリリンク
#ifdef _WIN32
#pragma comment(lib, "crypt32.lib")			// Cryptography API
#pragma comment(lib, "bcrypt.lib")			// bcryptライブラリ
#pragma comment(lib, "ws2_32.lib")			// Winsock2ライブラリ
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "secur32.lib")		// SSL/TLSライブラリ)
#pragma comment(lib, "synchronization.lib") // 同期オブジェクト

#ifdef _DEBUG
#pragma comment(lib, "msvcrtd.lib")	// MSVCデバッグランタイム
#else
#pragma comment(lib, "msvcrt.lib")	// MSVCリリースランタイム
#endif // _DEBUG
#endif

// 静的メンバ変数の定義
bool WebClient::s_wsaInitialized = false;
std::mutex WebClient::s_wsaMutex;

WebClient::WebClient() : m_isConnected(false) {
	//WSAStartupの呼び出し
	std::lock_guard<std::mutex> lock(s_wsaMutex);
	if (!s_wsaInitialized) {
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			throw std::runtime_error("WSAStartup failed with error: " + std::to_string(result));
		}
		s_wsaInitialized = true;
	}

	//コールバック関数の初期化
	SetupWebSocketCallbacks();
}

WebClient::~WebClient() {
	//切断
	Disconnect();
}

bool WebClient::Connect(const std::string& url) {
	//URL設定
	m_webSocket.setUrl(url);

	//接続開始
	m_webSocket.start();

	//接続完了まで待機（タイムアウト付き）
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	return m_isConnected.load();
}

void WebClient::Disconnect() {
	//接続中なら切断
	if (m_isConnected.load()) {
		m_webSocket.stop();
		m_isConnected = false;
	}
}

bool WebClient::IsConnected() const {
	//接続状態の取得
	return m_isConnected.load();
}

bool WebClient::SendMessageClient(const json& message) {
	//接続状態を確認
	if (!IsConnected()) {
		return false;
	}

	try {
		//JSONオブジェクトを文字列に変換して送信
		std::string messageStr = message.dump();
		m_webSocket.send(messageStr);
		return true;
	} catch (const std::exception& e) {
		//エラーをコールバックに通知
		if (m_onError) {
			m_onError("JSON serialization error: " + std::string(e.what()));
		}
		return false;
	}
}

bool WebClient::SendMessageClient(const std::string& message) {
	//接続状態を確認
	if (!IsConnected()) {
		return false;
	}
	//文字列を送信
	m_webSocket.send(message);
	return true;
}

void WebClient::SetOnConnected(OnConnectedCallBack callback) {
	//接続成功時のコールバック関数を設定
	m_onConnected = callback;
}

void WebClient::SetOnDisconnected(OnDisconnectedCallBack callback) {
	//切断時のコールバック関数を設定
	m_onDisconnected = callback;
}

void WebClient::SetOnMessage(OnMessageCallBack callback) {
	//メッセージ受信時のコールバック関数を設定
	m_onMessage = callback;
}

void WebClient::SetOnError(OnErrorCallBack callback) {
	//エラー発生時のコールバック関数を設定
	m_onError = callback;
}

void WebClient::ProcessMessages() {
	//メッセージキューからメッセージを取り出して処理
	std::lock_guard<std::mutex> lock(m_queueMutex);

	while (!m_messageQueue.empty()) {
		json message = m_messageQueue.front();
		m_messageQueue.pop();

		if (m_onMessage) {
			m_onMessage(message);
		}
	}
}

void WebClient::EnableAutomaticReconnection(bool enable) {
	//自動再接続の有効化・無効化
	if (enable) {
		m_webSocket.enableAutomaticReconnection();
	} else {
		m_webSocket.disableAutomaticReconnection();
	}
}

void WebClient::SetReconnectionInterval(int intervalMs) {
	//再接続間隔の設定（ミリ秒）
	m_webSocket.setMaxWaitBetweenReconnectionRetries(intervalMs);
}

void WebClient::SetExtraHeaders(const std::map<std::string, std::string>& headers) {
	//追加のHTTPヘッダーの設定
	ix::WebSocketHttpHeaders httpHeaders;
	for (const auto& header : headers) {
		httpHeaders[header.first] = header.second;
	}
	m_webSocket.setExtraHeaders(httpHeaders);
}

void WebClient::SetupWebSocketCallbacks() {
	//ixwebsocketのコールバック関数設定
	m_webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
		HandleMessage(msg);
		});
}

void WebClient::HandleMessage(const ix::WebSocketMessagePtr& msg) {
	//メッセージの種類に応じて処理
	switch (msg->type) {
		case ix::WebSocketMessageType::Open:
			//接続成功処理
			m_isConnected = true;
			std::cout << "Connected to " << msg->openInfo.uri << std::endl;
			if (m_onConnected) {
				m_onConnected();
			}

			//ゲームクライアントとしてサーバーに接続したことを通知
			{
				json connectMessage;
				connectMessage["type"] = "identify";
				connectMessage["role"] = "game";
				SendMessageClient(connectMessage);
			}

			break;
		case ix::WebSocketMessageType::Close:
			//切断処理
			m_isConnected = false;
			std::cout << "Disconnected: " << msg->closeInfo.code << " - " << msg->closeInfo.reason << std::endl;
			if (m_onDisconnected) {
				m_onDisconnected();
			}
			break;
		case ix::WebSocketMessageType::Message:
			//メッセージ受信処理
			try {
				//JSON文字列としてパース
				json message = json::parse(msg->str);
				{
					std::lock_guard<std::mutex> lock(m_queueMutex);
					m_messageQueue.push(message);
				}
			} catch (const json::parse_error& e) {
				//JSON解析エラー
				std::cerr << "JSON parse error: " << e.what() << std::endl;
				if (m_onError) {
					m_onError("JSON parse error: " + std::string(e.what()));
				}
			}
			break;
		case ix::WebSocketMessageType::Error:
			//エラー処理
			std::cerr << "WebSocket error: " << msg->errorInfo.reason << std::endl;
			if (m_onError) {
				m_onError("WebSocket error: " + msg->errorInfo.reason);
			}
			break;
		default:
			break;
	}
}
