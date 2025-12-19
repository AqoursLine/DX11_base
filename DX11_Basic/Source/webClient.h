#pragma once

//#include <x64-windows-static/ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocket.h>
#include <functional>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class WebClient {
public:
	//コールバック関数の型定義
	using OnConnectedCallBack = std::function<void()>;
	using OnDisconnectedCallBack = std::function<void()>;
	using OnMessageCallBack = std::function<void(const json& message)>;
	using OnErrorCallBack = std::function<void(const std::string& error)>;

	WebClient();
	~WebClient();

	//接続・切断
	bool Connect(const std::string& url);	//WebSocketサーバーに接続
	void Disconnect();						//WebSocketサーバーから切断
	bool IsConnected() const;				//接続状態の取得

	//JSONメッセージの送信
	bool SendMessageClient(const json& message);	//JSONオブジェクトを送信
	bool SendMessageClient(const std::string& message); //文字列を送信

	//コールバック関数の登録
	void SetOnConnected(OnConnectedCallBack callback);			//接続成功時のコールバック関数
	void SetOnDisconnected(OnDisconnectedCallBack callback);	//切断時のコールバック関数
	void SetOnMessage(OnMessageCallBack callback);				//メッセージ受信時のコールバック関数
	void SetOnError(OnErrorCallBack callback);					//エラー発生時のコールバック関数

	//メッセージ処理
	void ProcessMessages(); //受信メッセージの処理

	//オプション設定
	void EnableAutomaticReconnection(bool enable = true);			//自動再接続の有効化・無効化
	void SetReconnectionInterval(int intervalMs);					//再接続間隔の設定（ミリ秒）
	void SetExtraHeaders(const std::map<std::string, std::string>& headers);	//追加HTTPヘッダーの設定

private:
	static bool s_wsaInitialized; //WSA初期化フラグ
	static std::mutex s_wsaMutex; //WSA初期化の排他制御

	ix::WebSocket m_webSocket; //WebSocketクライアント
	std::atomic<bool> m_isConnected; //接続状態

	//受信メッセージキュー
	std::queue<json> m_messageQueue;
	std::mutex m_queueMutex; //メッセージキューの排他制御

	//コールバック関数
	OnConnectedCallBack m_onConnected;
	OnDisconnectedCallBack m_onDisconnected;
	OnMessageCallBack m_onMessage;
	OnErrorCallBack m_onError;

	//内部処理
	void SetupWebSocketCallbacks(); //WebSocketのコールバック関数設定
	void HandleMessage(const ix::WebSocketMessagePtr& msg); //メッセージ処理

};
