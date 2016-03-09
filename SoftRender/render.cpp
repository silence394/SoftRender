// *****************************************************
// For learn soft render.
// *****************************************************

#include "stdio.h"
#include "stdlib.h"

#include "windows.h"
#include "tchar.h"

#define WINDOW_CLASS_NAME L"soft render"
#define WINDOW_TITLE L"soft render app"

HWND	gHwnd;
HDC		gWindowDc;
char*	gFrameBuffer;
long	gScreenPitch;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
bool WindowInit( void* param = NULL );

int main()
{
	WindowInit();

	MSG msg;

	while(1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
	case WM_CREATE:
		return 0;
		break;
	case WM_KEYDOWN:
		{
			return 0;
		}
		break;
	case WM_KEYUP:
		{
			return 0;
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool WindowInit( void* param /* = NULL */ )
{
	WNDCLASS winclass;
	HDC hdc;
	RECT rect = {0, 0, 800, 600};


	winclass.style			= CS_BYTEALIGNCLIENT;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= GetModuleHandle(NULL);
	winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName	= WINDOW_CLASS_NAME;

	if (!RegisterClass(&winclass))
		return false;

	if (!(gHwnd = CreateWindow(WINDOW_CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, winclass.hInstance, NULL)))
	{
		return false;
	}

	hdc = GetDC(gHwnd);
	gWindowDc = CreateCompatibleDC(hdc);
	ReleaseDC(gHwnd, hdc);

	AdjustWindowRect(&rect, GetWindowLong(gHwnd, GWL_STYLE), 0 );

	int x1 = rect.right - rect.left;
	int y1 = rect.bottom - rect.top;

	int cx, cy;
	cx = (GetSystemMetrics(SM_CXSCREEN) - x1) / 2;
	cy = (GetSystemMetrics(SM_CYSCREEN) - y1) / 2;

	if (cy < 0)
		cy = 0;

	SetWindowPos(gHwnd, NULL, cx, cy, x1, y1, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));

	ShowWindow( gHwnd, SW_NORMAL );
	
	return true;
}