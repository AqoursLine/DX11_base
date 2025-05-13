#include "main.h"
#include <chrono>
#include <thread>
#include "manager.h"

//プロシージャ
LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
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

	//FPS制御
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	//変数
	TimePoint lastFrameTime = Clock::now();
	double targetFPS = 60.0f;
	double frameTime = (1.0 / 60.0);

	//マネージャークラス
	Manager* manager = new Manager();

	//メッセージクラス
	MSG msg;

	//メインループ
	while (true) {
		//メッセージ処理
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//終了メッセージ
			if (msg.message == WM_QUIT) {
				break;
			}
		}

		//現在時間取得
		TimePoint currentTime = Clock::now();
		//deltaTime計算
		double deltaTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();

		//フレームレート制限
		if (deltaTime < frameTime) {
			auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(frameTime - deltaTime));
			std::this_thread::sleep_for(sleepTime);

			//正確なdeltaTimeを再計算
			currentTime = Clock::now();
			deltaTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();
		}

		//秒数更新
		lastFrameTime = currentTime;

		//deltaTimeの上限設定
		deltaTime = min(deltaTime, 0.1);

		//ゲーム更新
		bool isFinished = manager->Update(deltaTime);

		if (isFinished) {
			break;
		}

		//ゲーム描画
		manager->Draw();

		//ゲーム後処理
		manager->CleanUp();


	}

	//マネージャークラス終了

	timeEndPeriod(1);

	//ウィンドウ登録解除
	UnregisterClass(className.c_str(), wc.hInstance);

	//COMライブラリ終了
	CoUninitialize();

	return static_cast<int>(msg.wParam);
}
