#include "main.h"
#include "timer.h"
#include "renderer.h"
#include "system.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "imguiSystem.h"

//
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//グローバル変数
HWND g_hWnd = nullptr;

#define WNDOW_CLASS_NAME L"DirectX11Window"

#ifdef _DEBUG
//コンソールカーソルを左上に移動
void MoveConsoleCursorToTopLeft() {
	COORD coord = { 0, 0 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//コンソールの内容をクリア
void ClearConsole() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count;
	DWORD cellCount;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) return;
	//コンソールのバッファ情報を取得
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;
	//コンソールの内容を空白で塗りつぶし
	if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, { 0, 0 }, &count)) return;
	//属性情報もリセット
	if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, { 0, 0 }, &count)) return;
	//カーソルを左上に移動
	SetConsoleCursorPosition(hConsole, { 0, 0 });
}

#endif // _DEBUG


void ErrorMessage(const std::wstring& msg, HRESULT hr) {
	//エラーメッセージを表示

	//HRESULTを文字列に変換
	std::wstringstream ss;
	ss << L"Error ID: 0x" << std::hex << std::uppercase << hr << L"\n";
	std::wstring errorMsg = ss.str();

	hr = MessageBox(g_hWnd, errorMsg.c_str(), msg.c_str(), MB_OK | MB_ICONERROR);
	if (SUCCEEDED(hr)) {
		DestroyWindow(g_hWnd);
	}

}

HWND GetHwnd() {
	return g_hWnd;
}

//プロシージャ
LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// imgui用のメッセージ処理
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	//メッセージ分岐
	switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				DestroyWindow(hWnd);
			}
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

//	_CrtSetBreakAlloc(65974);

	//ウィンドウクラス作成
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = WNDOW_CLASS_NAME;
	wc.hIconSm = nullptr;
	
	//ウィンドウクラス登録
	RegisterClassEx(&wc);

	//ディスプレイサイズ取得
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int windowPosX = (screenWidth - SCREEN_WIDTH) / 2;
	int windowPosY = (screenHeight - SCREEN_HEIGHT) / 2;

	//ウィンドウ作成
	g_hWnd = CreateWindowEx(0, WNDOW_CLASS_NAME, L"とりあえず", WS_POPUP | WS_VISIBLE, windowPosX, windowPosY, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, nullptr, hInstance, nullptr);

	//COMライブラリ初期化
	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	//DirectX11初期化
	Renderer::CreateInstance();
	RENDERER.Initialize(g_hWnd);

	//ウィンドウ表示
	ShowWindow(g_hWnd, nCmdShow);
	//ウィンドウ更新
	UpdateWindow(g_hWnd);

	//システムクラス初期化
	System::CreateInstance();
	bool isSystemInitialized = SYSTEM.Initialize();
	if (!isSystemInitialized) {
		ErrorMessage(L"システム初期化に失敗しました", E_FAIL);
		return -1;
	}

	//タイマークラス
	Timer timer;

	timer.Reset();
	timer.Start();

	//メッセージクラス
	MSG msg;

	//経過時間
	double elapsedTime = 0.0f;

	//フレームレート
	int frameRate = 60;

	//フレーム時間
	double frameTime = 1.0 / frameRate;

	//ループフラグ
	bool isLoop = true;

	int frameCount = 0;

	//メインループ
	while (isLoop) {
		//メッセージ処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//終了メッセージ
			if (msg.message == WM_QUIT) {
				break;
			}
		}

		timer.Tick();
		//経過時間に加算
		elapsedTime += timer.GetDeltaTime();

		//経過時間がフレームレートを超えたら
		if (elapsedTime >= frameTime) {
			//フレーム時間を引く
			elapsedTime = 0;
			if (SYSTEM.Excute()) {
				isLoop = false;
				break;
			}
		}
	}

	//システムクラス終了
	SYSTEM.Finalize();
	//システムクラス破棄
	System::DestroyInstance();

	//DirectX11終了
	RENDERER.Finalize();
	Renderer::DestroyInstance();

	//ウィンドウ登録解除
	UnregisterClass(WNDOW_CLASS_NAME, wc.hInstance);

	//COMライブラリ終了
	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

