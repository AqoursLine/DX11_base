#pragma once
//#define _WINSOCKAPI_ 

#include <ixwebsocket/IXWebSocket.h>
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


void ErrorMessage(std::wstring msg, HRESULT hr);

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

