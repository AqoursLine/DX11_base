#include "titleMenuIconQuit.h"

#include "main.h"
#include "system.h"
#include "manager.h"

void TitleMenuIconQuit::OnDecide() {
	HWND hwnd = GetHwnd();
	SendMessage(hwnd, WM_CLOSE, 0, 0);
}
