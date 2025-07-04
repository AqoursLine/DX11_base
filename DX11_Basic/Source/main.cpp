#include "main.h"
#include "System/timer.h"
#include "DX11/renderer.h"
#include "System/system.h"


HWND g_hWnd = nullptr;

void ErrorMessage(std::wstring msg, HRESULT hr) {
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

#ifdef _DEBUG
	// デバッグ用のコンソールを作成
	AllocConsole();

	//標準出力ストリームをコンソールにリダイレクト
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
	freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

#endif // _DEBUG


	//DirectX11初期化
	Renderer::CreateInstance();
	RENDERER.Initialize(hWnd);

	//システムクラス初期化
	System::CreateInstance();
	bool isSystemInitialized = SYSTEM.Initialize();
	if (!isSystemInitialized) {
		ErrorMessage(L"システム初期化に失敗しました", E_FAIL);
		return -1;
	}

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
		while (elapsedTime >= frameTime) {
			std::cout << "Frame Time:" << frameTime << ";" << "Frame Time: " << elapsedTime << " seconds" << std::endl;
			//フレーム時間を引く
			elapsedTime -= frameTime;
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
	UnregisterClass(className.c_str(), wc.hInstance);

	//COMライブラリ終了
	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

