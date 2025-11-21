#pragma once
#include "main.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class ImguiSystem {
public:
	ImguiSystem() = default;
	~ImguiSystem() { Finalize(); }

	bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
	void Finalize();

	void Begin();
	void End();

private:

};
