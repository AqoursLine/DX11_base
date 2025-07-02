#pragma once

#pragma once

//#define WIN32_LEAN_AND_MEAN

#pragma comment(lib, "ixwebsocket.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")    // CertOpenStore, CertCloseStore等
#pragma comment(lib, "secur32.lib")    // SSL/TLS機能
#pragma comment(lib, "bcrypt.lib")     // 暗号化プリミティブ
#pragma comment(lib, "advapi32.lib")

class GameWebSocketClient {
public:
	GameWebSocketClient();
	~GameWebSocketClient();

	bool Connect(const std::string& url);

	void SendMessage(const std::string& message);

	void Disconnect();

	bool IsConnected() const {
		return m_connected;
	}
private:
	ix::WebSocket m_client;
	bool m_connected;
	std::string m_url;
};

