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
HBITMAP	gHBitMap;
HBITMAP	gOldBitMap;
unsigned char*	gScreenBuffer;
long	gScreenPitch;

unsigned int** gFrameBuffer;

int gWidth, gHeight = 0;
long gLPitch = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
bool WindowInit( int w, int h );
int GameMain();

int main()
{
	WindowInit(800, 600);

	//gScreenBuffer
	//Init gFramebuffer.
	{
		char* ptr = (char*) malloc(gWidth * gHeight * 4);
		gFrameBuffer = (unsigned int**) ptr;
		for (int y = 0; y < gHeight; y ++)
		{
			gFrameBuffer[y] = (unsigned int*) (gScreenBuffer + gWidth * y * 4);
		}
		
	}

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
		GameMain();
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

bool WindowInit(int w, int h)
{
	WNDCLASS winclass;
	HDC hdc;
	LPVOID ptr;
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

	BITMAPINFO bitinfo;

	bitinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitinfo.bmiHeader.biWidth = w;
	bitinfo.bmiHeader.biHeight = -h;
	bitinfo.bmiHeader.biPlanes = 1;
	bitinfo.bmiHeader.biBitCount = 32;
	bitinfo.bmiHeader.biCompression = BI_RGB;
	bitinfo.bmiHeader.biSizeImage = w * h * 4;
	bitinfo.bmiHeader.biXPelsPerMeter = 0;
	bitinfo.bmiHeader.biYPelsPerMeter = 0;
	bitinfo.bmiHeader.biClrUsed = 0;
	bitinfo.bmiHeader.biClrImportant = 0;

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

	gHBitMap = CreateDIBSection(gWindowDc, &bitinfo, DIB_RGB_COLORS, &ptr, 0, 0);
	if (gHBitMap == NULL)
		return false;

	gOldBitMap = (HBITMAP) SelectObject(gWindowDc, gHBitMap);

	gScreenBuffer = (unsigned char*) ptr;
	gWidth = w;
	gHeight = h;
	gLPitch = w * 4;

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

	//Ã¿ÏñËØ4¸ö×Ö½Ú
	memset(gScreenBuffer, 0, w * h * 4);
	
	return true;
}

int GameMain()
{
	//Clear framebuffer.
	for (int y = 0; y < gHeight; y ++)
	{
		for ( int x = 0; x < gWidth; x ++ )
		{
			gFrameBuffer[y][x] = 0xff000000;
		}
	}

	for (int i = 0; i < 1000; i ++)
	{
		int xx = rand() % (gWidth - 1);
		int yy = rand() % (gHeight - 1);

		int r = rand()%255;
		int g = rand()%255;
		int b = rand()%255;
		gFrameBuffer[yy][xx] = 0xff<< 24 + r << 16 + g << 8 + b;
	}
	
	HDC hdc = GetDC(gHwnd);
	BitBlt( hdc, 0, 0, gWidth, gHeight, gWindowDc, 0, 0, SRCCOPY);
	ReleaseDC( gHwnd, hdc );
	
	Sleep(30);
	return 0;
}