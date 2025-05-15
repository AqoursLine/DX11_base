#pragma once

#include <Windows.h>
#include <iostream>

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <DirectXMath.h>
using namespace DirectX;


constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
