#include "imguiSystem.h"

bool ImguiSystem::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) {
	// ImGuiコンテキストの作成
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // キーボードナビゲーションを有効化

	// ImGuiスタイルの設定
	ImGui::StyleColorsDark();
	// Win32およびDirectX11のバックエンド初期化
	if (!ImGui_ImplWin32_Init(hwnd)) {
		return false;
	}
	if (!ImGui_ImplDX11_Init(device, context)) {
		return false;
	}
	return true;
}

void ImguiSystem::Finalize() {
	// ImGuiバックエンドの終了
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	// ImGuiコンテキストの破棄
	ImGui::DestroyContext();
}

void ImguiSystem::Begin() {
	// 新しいフレームの開始
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImguiSystem::End() {
	// ImGuiのレンダリング
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
