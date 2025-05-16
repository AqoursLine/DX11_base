#include "main.h"
#include "manager.h"
#include "timer.h"


HWND g_hWnd = nullptr;

void ErrorMessage(std::wstring msg, HRESULT hr) {
	std::wstring errorId = L"Error ID: " + std::to_wstring(hr) + L"\n";

	hr = MessageBox(g_hWnd, errorId.c_str(), msg.c_str(), MB_OK | MB_ICONERROR);
	if (SUCCEEDED(hr)) {
		DestroyWindow(g_hWnd);
	}

}

//プロシージャ
LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
	std::wstring className = L"DirectX11";

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
	wc.lpszClassName = className.c_str();
	wc.hIconSm = nullptr;
	
	//ウィンドウクラス登録
	RegisterClassEx(&wc);

	//サイズ徴性
	RECT rc = { 0,0,(LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	//ウィンドウ作成
	HWND hWnd = CreateWindowEx(0, className.c_str(), L"とりあえず", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

	//COMライブラリ初期化
	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	//ウィンドウ表示
	ShowWindow(hWnd, nCmdShow);
	//ウィンドウ更新
	UpdateWindow(hWnd);

	g_hWnd = hWnd;

	//マネージャークラス
	Manager* manager = new Manager();
	bool isInitialized = manager->Initialize();

	if (!isInitialized) {
		ErrorMessage(L"マネージャークラスの初期化に失敗しました", E_FAIL);
	}

	Timer timer;

	timer.Reset();
	timer.Start();

	//メッセージクラス
	MSG msg;

	//メインループ
	while (true) {
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
		if (manager->Update(timer.GetDeltaTime())) {
			break;
		}
		manager->Draw();
		manager->CleanUp();

	}

	//マネージャークラス終了
	manager->Finalize();
	delete manager;

	//ウィンドウ登録解除
	UnregisterClass(className.c_str(), wc.hInstance);

	//COMライブラリ終了
	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

