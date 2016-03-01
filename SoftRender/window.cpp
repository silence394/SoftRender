// Window.cpp 

#include "windows.h"
#include "soft3dlib.h"

#define WINDOW_CLASS_NAME	L"SoftRender"
#define WINDOW_TITLE		L"soft render app"
#define WINDOW_WIDTH		320
#define WINDOW_HEIGHT		240

int GameInit(void *param = NULL);
int GameShutdown(void *param = NULL);
int GameMain(void *param = NULL);

HWND		gMainWindowHandle = NULL;
HINSTANCE	gInstance = NULL;
char		gBuffer[256];

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	PAINTSTRUCT	ps;
	HDC hdc;

	switch(msg)
	{
		case WM_CREATE:
		{
			return 0;
		}
		break;

		case WM_PAINT:
		{
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}

	return (DefWindowProc(hwnd, msg, wparam, lparam));
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hpreinstance, LPSTR lcmdline, int ncmdshow)
{
	WNDCLASS	winclass;
	HWND		hwnd;
	MSG			msg;
	HDC			hdc;
	PAINTSTRUCT	ps;

	winclass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hinstance;
	winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName	= WINDOW_CLASS_NAME;

	if (!RegisterClass(&winclass))
		return 0;

	if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
							0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hinstance, NULL)))
		return 0;

	gMainWindowHandle = hwnd;
	gInstance = hinstance;

	ShowWindow(gMainWindowHandle, SW_SHOW);

	GameInit();

	while(1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		GameMain();
	}

	GameShutdown();

	return msg.wParam;
}

int GameMain(void *param /* = NULL */)
{
	return 0;
}

int GameInit(void *param /* = NULL */)
{
	return 0;
}

int GameShutdown(void *param /* = NULL */)
{
	return 0;
}