#pragma once

#define SCREE_WIDTH 640
#define	SCREEN_HEIGHT 320
#define SCREE_BPP 8
#define MAX_COLORS_PALETTE 256
    
#define BITMAP_ID 0X4D42
//...

#define PI ((float)3.141592654f)
#define PI2 ((float)6.283185307f)
#define PI_INV ((float)0.318309886f)


#define KEYDOWN(keycode)	((GetAsyncKeyState(keycode) & 0x8000 ) ? 1 : 0 )
#define KEYUP(key)			((GetAsyncKeyState(keycode) & 0x8000 ) ? 1 : 0 )

#define RGB32BIT(a, r, g, b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define DEG2RAD(ang) ((ang) * PI / 180.0f)
#define RAD2DEG(rads) ((rads) * 180.0f / PI)

#define RAND_RANGE(x, y) ((x) + rand() * ((y) - (x) + 1))

typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned int QUAT;
typedef unsigned int UINT;

int DrawInit(int width, int height, int bpp, int windowed = 0);
int DrawShutdown(void);

int DrawFlip(void);

DWORD GetClock(void);
DWORD StartClock(void);
DWORD WaitClock(DWORD count);

int DrawClipLine(int x0, int y0, int x1, int y1, int color, UCHAR* buffer, int lpitch);
int DrawClipLine16(int x0, int y0, int x1, int y1, int color, UCHAR* buffer, int lpitch);
int ClipLine(int& x1, int& y1, int& x2, int& y2);

int DrawLine(int x0, int y0, int x1, int y1, int color, UCHAR* vbstart, int lpitch);
int DrawLine16(int x0, int y0, int x1, int y1, int color, UCHAR* vbstart, int lpitch);

int DrawPixel(int x, int y, int color, UCHAR* videobuffer, int lpich);
int DrawRectangle(int x1, int y1, int x2, int y2, int color, LPDIRECTDRAWSURFACE7 lpdds);
