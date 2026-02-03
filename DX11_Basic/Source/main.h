#pragma once

#define NOMINMAX

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include <DirectXMath.h>
using namespace DirectX;

#include "DirectXTex.h"

#ifdef _DEBUG
#pragma comment (lib, "DirectXTex_Debug.lib")
#else
#pragma comment (lib, "DirectXTex_Release.lib")
#endif // _DEBUG

#include "gwVector.h"

void ErrorMessage(const std::wstring& msg, HRESULT hr);

void SetActivatedImGui(bool isActive);
bool IsActivatedImGui();

HWND GetHwnd();

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
