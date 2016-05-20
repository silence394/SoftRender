// *****************************************************
// To learn soft render.
// *****************************************************

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "windows.h"
#include "tchar.h"

#define WINDOW_CLASS_NAME L"soft render"
#define WINDOW_TITLE L"soft render app"

#define PI 3.1415926f
typedef unsigned int uint;
#define MIN(A, B, C) (A < B) ? (A < C ? A : C) : (B < C ? B : C) 
#define MAX(A, B, C) (A > B) ? (A > C ? A : C) : (B > C ? B : C)
#define MID(x, nmin, nmax) ((x < nmin) ? nmin : (x < nmax ? x : nmax))

#define IsKeyDown(keycode) ((GetAsyncKeyState(keycode) & 0x8000) ? 1 : 0)
#define IsKeyUp(keycode) ((GetAsyncKeyState(keycode) & 0x8000 ? 0 : 1))

HWND	gHwnd;
HDC		gWindowDc;
HBITMAP	gHBitMap;
HBITMAP	gOldBitMap;

uint gTexture[256][256];
unsigned char* gScreenBuffer;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
bool OnKeyDown(uint key);

enum RenderMethod
{
	wireFrame	= 1,
	color		= 2,
	texture		= 4,
};

struct Vector3
{
	float x;
	float y;
	float z;
};

struct Vector4
{
	float x, y, z, w;
};

struct TextCoord
{
	float u, v;
};

struct Color
{
	float r, g, b, a;
};

struct Vertex
{
	Vector3 pos;
	TextCoord texture;
	Color color;
};

struct Edge
{
	float x;
	float dx;
	int ymax;
	float w;
	float dw;
	Vertex v;
	Vertex dv;
	struct Edge* next;
};

struct Matrix44
{
	float m[4][4];
};

struct RenderDevice
{
	Matrix44	mWorld;
	Matrix44	mView;
	Matrix44	mProject;

	Matrix44	mTransform;
	float		mWidth;
	float		mHeight;

	uint**		mFrameBuffer;
	float**		mZBuffer;
	uint**		mTexture;
	int			mTexWidth;
	int			mTexHeight;

	// Camera.
	Vector3 eye;
	Vector3 look;
	Vector3 up;
	float	fov;

	uint	mRenderMethod;
};

struct AmbientLight
{
	Color	color;
};

struct DirectLight
{
	Color	color;
	Vector3 dir;
};

struct PointLight
{
	Color	color;
	Vector3	pos;
	float	range;
	float	kc;
	float	kl;
	float	kq;
};

struct Material
{
	Color		ambient;
	Color		diffuse;
	Color		specular;
	Color		emissive;
	float		power;
};

struct Material1
{
	Color color;
	float ka;
	float kd;
	float ks;
	float power;
};

AmbientLight	gAmbientLight;
DirectLight		gDirectLight;
PointLight		gPointLight;

Material		gMaterial;

bool WindowInit(int w, int h);
void DeviceInit(RenderDevice* rd, int w, int h, unsigned char* fb);
void TextureInit(RenderDevice* rd, int w, int h);
void InitLights( );
void InitMaterial( );
int	GameMain();
bool CheckInCVV(const Vector4& v);
void UpdateTransform(RenderDevice* rd);
void DrawCube(RenderDevice* rd);
void DrawPlane(RenderDevice* rd, int a, int b, int c, int d);
void DrawPrimitive(RenderDevice* rd, const Vertex& v1, const Vertex& v2, const Vertex& v3);
void ViewPort(RenderDevice* rd, Vector4& v1, const Vector4& v2);

void DrawPixel(RenderDevice* rd, int x, int y, unsigned int color);
void DrawLine(RenderDevice* rd, int x1, int y1, int x2, int y2, unsigned int color);

void MatrixSetIndentity(Matrix44& mat);
void MatrixApply(Vector4& v1, const Vector3& v2, const Matrix44& mat);
void MatrixMulRight( Matrix44& m, Matrix44& a, Matrix44& b );
void MatrixSetRotation(Matrix44& m, const Vector3& n, float r);
void MatrixSetProjection(Matrix44& m, float fov, float aspect, float nz, float zf);
void MatrixSetCamear(Matrix44& mat, const Vector3& eye, const Vector3& look, const Vector3& up);

void Vector3Add(Vector3& v, const Vector3& va, const Vector3& vb);
void Vector3Mul(Vector3& v, float n);
void Vector3Sub(Vector3&v, const Vector3& va, const Vector3&vb);
float Vector3Dot(const Vector3& va, const Vector3&vb);
void Vector3Cross(Vector3& v, const Vector3& va, const Vector3&vb);
float Vector3Length(Vector3& v);
void Vector3Normalize(Vector3& v);

void ColorMul(Color& c, float n);
void ColorAdd(Color& c1, Color& c2);
void ColorDot(Color& c1, const Color& c2, const Color& c3);
void ColorDot(Color& c1, const Color& c2);

void GetVector3InVector4(Vector3& v, const Vector4& v4 );

float Interpolate(float x1, float x2, float t);

void Vector3Interpolate(Vector3& v1, const Vector3& v2, const Vector3& v3, float t);
void Vector3Division(Vector3&v1, const Vector3& v2, const Vector3& v3, float w);

void VertexInterpolate(Vertex& v1, const Vertex& v2, const Vertex& v3, float t);
void VertexDivision(Vertex& v1, const Vertex& v2, const Vertex& v3, float w);
void VertexAdd(Vertex& v1, Vertex& v2);

void InsertEdge(Edge* list, Edge* edge);
//void InitNET(Vector3* points, Vertex* vertexs ,int num, Edge* edge[], int minscanline);
void InitNETs(Vector4& p1, const Vertex& v1, Vector4& p2, const Vertex& v2, Vector4& p3, const Vertex& v3, Edge* edges, int ymin);
void InitNET(Vector4&p1, const Vertex&v1, Vector4& p2, const Vertex& v2, Edge* edges, int minscanline);
void BuildAET(Edge* aet, Edge* net);
void UpdateAET(Edge* aet, int scanline);
void ResortAET(Edge* aet);
void DrawScanline(RenderDevice* rd, Edge* aet, int scanline, Vector3& normal);

void DeviceClear(RenderDevice* rd);
// 
// Vertex gMesh[8] = {
// 	{ {1.0f, 1.0f, 1.0f},	{0, 0}, {1.0f, 0.2f, 0.2f} },
// 	{ {1.0f, 1.0f, -1.0f},	{0, 1}, {0.2f, 1.0f, 0.2f} },
// 	{ {-1.0f, 1.0f, -1.0f}, {1, 1}, {0.2f, 0.2f, 1.0f} },
// 	{ {-1.0f, 1.0f, 1.0f}, {1, 0}, {1.0f, 0.2f, 1.0f} },
// 	{ {1.0f, -1.0f, 1.0f},	{0, 0}, {1.0f, 1.0f, 0.2f} },
// 	{ {-1.0f, -1.0f, 1.0f}, {0, 1}, {0.2f, 1.0f, 0.3f} },
// 	{ {-1.0f, -1.0f, -1.0f}, {1, 1}, {1.0f, 0.3f, 0.3f} },
// 	{ {1.0f, -1.0f, -1.0f}, {1, 0}, {.2f, 1.0f, 1.0f} },
// };

Vertex gMesh[8] = {
	{ {1.0f, 1.0f, 1.0f},	{0, 0}, {1.0f, 1.0f, 1.0f} },
	{ {1.0f, 1.0f, -1.0f},	{0, 1}, {1.0f, 1.0f, 1.0f} },
	{ {-1.0f, 1.0f, -1.0f}, {1, 1}, {1.0f,1.0f, 1.0f} },
	{ {-1.0f, 1.0f, 1.0f}, {1, 0}, {1.0f, 1.0f, 1.0f} },
	{ {1.0f, -1.0f, 1.0f},	{0, 0}, {1.0f, 1.0f, 1.0f} },
	{ {-1.0f, -1.0f, 1.0f}, {0, 1}, {1.0f, 1.0f, 1.0f} },
	{ {-1.0f, -1.0f, -1.0f}, {1, 1}, {1.0f, 1.0f, 1.0f} },
	{ {1.0f, -1.0f, -1.0f}, {1, 0}, {1.0f, 1.0f, 1.0f} },
};

RenderDevice gRenderDevice;
int main()
{
	int w = 800, h = 600;

	if (!WindowInit(w, h))
		return 1;

	DeviceInit(&gRenderDevice, w, h, gScreenBuffer);

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
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			OnKeyDown(wparam);
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

	memset(gScreenBuffer, 0, w * h * 4);

	return true;
}

void DeviceInit(RenderDevice* rd, int w, int h, unsigned char* fb)
{
	rd->mWidth = (float) w;
	rd->mHeight = (float) h;
	rd->mRenderMethod = RenderMethod::texture;

	{
		char* ptr = (char*) malloc(w * h * 4);
		rd->mFrameBuffer = (unsigned int**) ptr;
		rd->mZBuffer = (float**) malloc( sizeof(float) * h );

		for (int y = 0; y < h; y ++)
		{
			rd->mFrameBuffer[y] = (unsigned int*) (fb + w * y * 4);

			rd->mZBuffer[y] = (float*) malloc( sizeof(float) * w );
			memset(rd->mZBuffer[y], 'a', sizeof(float) * w);
		}
	}
	 
	TextureInit(rd, 256, 256);

	Vector3 eye = {2.0f, 2.0f, 2.0f};
	Vector3 look = { 0.0f, 0.0f, 0.0f};
	Vector3 up = {0.0f, 1.0f, 0.0f};
	rd->eye = eye;
	rd->look = look;
	rd->up = up;
	rd->fov = PI * 0.5f;
	MatrixSetCamear(rd->mView, eye, look, up);

	MatrixSetProjection(rd->mProject, rd->fov, (float) w / (float) h, 1.0f, 500.0f );

	MatrixSetIndentity(rd->mWorld);
	MatrixSetIndentity(rd->mTransform);

	// Init light.
	InitLights();
	
	// Init material;
	InitMaterial();
}

int times = 0;
int GameMain()
{
	//Clear framebuffer.
	DeviceClear(&gRenderDevice);

	Vector3 n = {0.0f, 1.0f, 0.0f};
	static float rotate = 0.0f;
	rotate = PI / 5400.0f * times++;
//	MatrixSetRotation(gRenderDevice.mWorld, n, rotate);

	UpdateTransform(&gRenderDevice);

	DrawCube(&gRenderDevice);

	HDC hdc = GetDC(gHwnd);
	BitBlt( hdc, 0, 0, (int) gRenderDevice.mWidth, (int) gRenderDevice.mHeight, gWindowDc, 0, 0, SRCCOPY);
	ReleaseDC( gHwnd, hdc );

	Sleep(1);
	return 0;
}

void DrawPixel(RenderDevice* rd, int x, int y, unsigned int color )
{
	if ( x < rd->mWidth && y < rd->mHeight )
	{
		rd->mFrameBuffer[y][x] = color;
	}
}

void DrawLine(RenderDevice* rd, int x1, int y1, int x2, int y2, unsigned int color)
{
	if (x1 == x2)
	{
		int dy = y1 <= y2 ? 1 : -1;
		if (y1 == y2)
			dy = 0;
		do
		{
			DrawPixel(rd, x1, y1, color );
			y1 += dy;
		}
		while(y1 != y2);
	}
	else 
	{
		int stepx = x1 <= x2 ? 1 : -1;
		int stepy = y1 <= y2 ? 1 : -1;
		int dx = stepx == 1 ? x2 - x1 : x1 - x2;
		int dy = stepy == 1 ? y2 - y1 : y1 - y2;
		int eps = 0;
		int t;

		if(dx >= dy)
		{
			t = y1;
			for (int x = x1; x != x2; x += stepx )
			{
				DrawPixel(rd, x, t, color);
				eps += dy;
				if ((eps << 1) >= dx)
				{
					eps -= dx;
					t += stepy;
				}
			}
		}
		else
		{
			t = x1;
			for (int y = y1; y != y2; y += stepy)
			{
				DrawPixel(rd, t, y, color);
				eps += dx;
				if ((eps << 1) >= dy)
				{
					eps -= dy;
					t += stepx;
				}
			}
		}
	}
}

void InsertEdge(Edge* list, Edge* edge)
{
	Edge *p, *q = list;
	p = q->next;
	while(p != NULL)
	{
		if (edge->x < p->x || (edge->x == p->x && edge->dx < p->dx))
			p = NULL;
		else
		{
			q = p;
			p = p->next;
		}
	}
	edge->next = q->next;
	q->next = edge;
}

void TextureInit(RenderDevice* rd, int w, int h)
{
	rd->mTexWidth = w;
	rd->mTexHeight = h;
	//rd->mTexture = (uint**) malloc(sizeof(uint*)*256);
	uint** ptr = (uint**) malloc(sizeof(uint*) * w * h);
	rd->mTexture = ptr;
	for (int j = 0; j < h; j ++ )
	{
		for ( int i = 0; i < w; i ++ )
		{
			int x = i / 32, y = j / 32;
			gTexture[j][i] = ((x + y) & 1) ? 0xffffffff : 0xff0093dd; 
		}
		rd->mTexture[j] = (uint*) gTexture[j];
	}
}

void InitLights()
{
	gAmbientLight.color.a = 1.0f;
	gAmbientLight.color.r = 1.0f;
	gAmbientLight.color.g = 1.0f;
	gAmbientLight.color.b = 1.0f;

	gDirectLight.color.a = 1.0f;
	gDirectLight.color.r = 0.5f;
	gDirectLight.color.g = 0.5f;
	gDirectLight.color.b = 0.5f;

	gDirectLight.dir.x = 1.0f;
	gDirectLight.dir.y = 1.0f;
	gDirectLight.dir.z = 0.0f;
	Vector3Normalize(gDirectLight.dir);

	gPointLight.color.a = 1.0f;
	gPointLight.color.r = 0.0f;
	gPointLight.color.g = 1.0f;
	gPointLight.color.b = 0.0f;

	gPointLight.pos.x = 0.0f;
	gPointLight.pos.y = 5.0f;
	gPointLight.pos.z = 0.0f;

	gPointLight.kc = 1.0f;
	gPointLight.kl = 1.0f;
	gPointLight.kq = 1.0f;

	gPointLight.range = 100.0f;
}

void InitMaterial()
{
// 	gMaterial.ambient.a = 1.0f;
// 	gMaterial.ambient.r = 1.0f;
// 	gMaterial.ambient.g = 0.5f;
// 	gMaterial.ambient.b = 0.31f;
	gMaterial.ambient.a = 1.0f;
	gMaterial.ambient.r = 1.0f;
	gMaterial.ambient.g = 1.0f;
	gMaterial.ambient.b = 1.0f;

	gMaterial.diffuse.a = 1.0f;
	gMaterial.diffuse.r = 1.0f;
	gMaterial.diffuse.g = 1.0f;
	gMaterial.diffuse.b = 1.0f;

	gMaterial.specular.a = 1.0f;
	gMaterial.specular.r = 0.5f;
	gMaterial.specular.g = 0.5f;
	gMaterial.specular.b = 0.5f;

	gMaterial.emissive.a = 1.0f;
	gMaterial.emissive.r = 0.5f;
	gMaterial.emissive.g = 0.5f;
	gMaterial.emissive.b = 0.5f;

	gMaterial.power = 32;
}

void BuildAET(Edge* aet, Edge* net)
{
	Edge *p = net->next;
	Edge *q;
	while(p != NULL)
	{
		q = p->next;
		InsertEdge(aet, p);
		p = q;
	}

	net->next = NULL;
}

void UpdateAET(Edge* aet, int scanline)
{
	Edge *p, *q = aet;
	p = q->next;
	while(p != NULL)
	{
		if (p->ymax == scanline)
		{
			q->next = p->next;
			p->next = NULL;
			free(p);
			p = q->next;
		}
		else
		{
			VertexAdd(p->v, p->dv);
			p->x += p->dx;
			p->w += p->dw;
			q = p;
			p = p->next;
		}
	}
}

void ResortAET(Edge* aet)
{
	Edge *q, *p = aet->next;
	aet->next = NULL;
	while(p != NULL)
	{
		q = p->next;
		InsertEdge(aet, p);
		p = q;
	}
}

void DrawScanline(RenderDevice* rd, Edge* aet, int scanline, Vector3& normal)
{
	Edge *p1, *p2;
	p1 = aet->next;
	while(p1 != NULL)
	{
		p2 = p1->next;

		int x1 = (int) (p1->x), x2 = (int) (p2->x);
		if (x1 != x2)
		{
			Vertex v = p1->v;
			Vertex dv;

			float width = (float) (x2 - x1);
			float inv = 1.0f / width;
			VertexDivision(dv, p1->v, p2->v, width);

			float dw = (p2->w - p1->w) * inv;
			float w = p1->w;

			for ( int j = x1; j < x2; j ++ )
			{
				float invw = 1.0f / w;
	
				if (rd->mZBuffer[scanline][j] < w )
				{
					rd->mZBuffer[scanline][j] = w;

					Vector3 worldpos = v.pos;
					Vector3Mul(worldpos, invw);

					if (rd->mRenderMethod & RenderMethod::color)
					{
						Color c = v.color;
						ColorMul(c, invw);


						// 计算环境光照。
						Color ambient = gAmbientLight.color;
						ColorDot(ambient, gMaterial.ambient);
						
						// 计算漫反射光照。对平行光和点光进行计算。
						Color diffuse;
						
						// 计算平行光
						diffuse = gDirectLight.color;
						float diff = max(-1.0f * Vector3Dot(normal, gDirectLight.dir), 0.0f);
						
						Color tc = ambient;

						if ( diff > 0.0f )
						{
							ColorMul(diffuse, diff);
							ColorDot(diffuse, gMaterial.diffuse);
							ColorAdd(tc, diffuse);
						}

						//ColorDot(c, ambient);
						ColorDot(c, tc);

						//ComputeAmbientLight();
						//ComputeDirectLight( );
						//ComputePointLight( );
// 						{
// 							Vector3 lightdir;
// 							Vector3Sub(lightdir, gPointLight.pos, worldpos);
// 							float dis = Vector3Length(lightdir);
// 
// 							float tt = 1.0f / ( gPointLight.kc + gPointLight.kl * dis + gPointLight.kc * dis * dis );
// 							
// 							Color tc = gPointLight.color;
// 							ColorMul(tc, tt);
// 							ColorDot(diffuse, tc);
// 						}


						

						
						// 计算镜面反射。


						//int A = (int) (c.a * 255.0f);
						int A = 255;
						int R = (int) (c.r * 255.0f);
						int G = (int) (c.g * 255.0f);
						int B = (int) (c.b * 255.0f);
						// 进行光照计算。拿到worldPos，进行光照计算就可以。

						DrawPixel(rd, j, scanline, (A<<24) | (R<<16) | (G<<8) | B);
					}

					if (rd->mRenderMethod & RenderMethod::texture)
					{
						int U = MID((int) (v.texture.u * 255.0f * invw + 0.5f), 0, 255);
						int V = MID((int) (v.texture.v * 255.0f * invw + 0.5f), 0, 255);
						// 转换为argb颜色值，进行光照计算。
						
						float temp = 1.0f / 255.0f;
						Color c;

						c.b = (rd->mTexture[V][U] & 0xff) * temp;
						c.g = ((rd->mTexture[V][U] >> 8) & 0xff) * temp;
						c.r = ((rd->mTexture[V][U] >> 16) & 0xff) * temp;
						c.a = ((rd->mTexture[V][U] >> 24) & 0xff) * temp;

						Color c1;
						ColorDot(c1, c, gAmbientLight.color);
						Color tc;
						ColorDot(tc, c1, gMaterial.ambient );

						int A = (int) (tc.a * 255.0f);
						int R = (int) (tc.r * 255.0f);
						int G = (int) (tc.g * 255.0f);
						int B = (int) (tc.b * 255.0f);

						DrawPixel(rd, j, scanline, (A<<24) | (R<<16) | (G<<8) | B);
					}
				} 

				VertexAdd(v, dv);
				w += dw;
			}
		}

		p1 = p2->next;
	}
}

void Vector3Add(Vector3& v, const Vector3& va, const Vector3& vb)
{
	v.x = va.x + vb.x;
	v.y = va.y + vb.y;
	v.z = va.z + vb.z;
}

void Vector3Mul(Vector3& v, float n)
{
	v.x *= n;
	v.y *= n;
	v.z *= n;
}

void Vector3Sub(Vector3&v, const Vector3& va, const Vector3&vb)
{
	v.x = va.x - vb.x;
	v.y = va.y - vb.y;
	v.z = va.z - vb.z;
}

float Vector3Dot(const Vector3& va, const Vector3&vb)
{
	return va.x * vb.x + va.y * vb.y + va.z * vb.z;
}

void Vector3Cross(Vector3& v, const Vector3& va, const Vector3&vb)
{
	v.x = va.y * vb.z - va.z * vb.y;
	v.y = va.z * vb.x - va.x * vb.z;
	v.z = va.x * vb.y - va.y * vb.x;
}

float Vector3Length(Vector3& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void Vector3Normalize(Vector3& v)
{
	float l = Vector3Length(v);
	if (l != 0.0f)
	{
		v.x /= l;
		v.y /= l;
		v.z /= l;
	}
}

void ColorMul(Color& c, float n)
{
	c.a *= n;
	c.r *= n;
	c.g *= n;
	c.b *= n;
}

void ColorAdd(Color& c1, Color& c2)
{
	c1.a += c2.a;
	c1.r += c2.r;
	c1.g += c2.g;
	c1.b += c2.b;
}

void ColorDot(Color& c1, const Color& c2, const Color& c3)
{
	c1.a = c2.a * c3.a;
	c1.r = c2.r * c3.r;
	c1.g = c2.g * c3.g;
	c1.b = c2.b * c3.b;
}

void ColorDot(Color& c1, const Color& c2)
{
	c1.a *= c2.a;
	c1.r *= c2.r;
	c1.g *= c2.g;
	c1.b *= c2.b;
}

void GetVector3InVector4(Vector3& v, const Vector4& v4)
{
	v.x = v4.x;
	v.y = v4.y;
	v.z = v4.z;
}

float Interpolate(float x1, float x2, float t)
{
	return x1 + (x2 - x1) * t;
}

void Vector3Interpolate(Vector3& v1, const Vector3& v2, const Vector3& v3, float t)
{
	v1.x = Interpolate(v2.x, v3.x, t);
	v1.y = Interpolate(v2.y, v3.y, t);
	v1.z = Interpolate(v2.z, v3.z, t);
}

void Vector3Division(Vector3&v1, const Vector3& v2, const Vector3& v3, float w)
{
	float inv = 1.0f / w;
	v1.x = (v3.x - v2.x) * inv;
	v1.y = (v3.y - v2.y) * inv;
	v1.z = (v3.z - v2.z) * inv;
}

void VertexInterpolate(Vertex& v1, const Vertex& v2, const Vertex& v3, float t)
{
	Vector3Interpolate(v1.pos, v2.pos, v3.pos, t);

	v1.color.a = Interpolate(v2.color.a, v3.color.a, t);
	v1.color.r = Interpolate(v2.color.r, v3.color.r, t);
	v1.color.g = Interpolate(v2.color.g, v3.color.g, t);
	v1.color.b = Interpolate(v2.color.b, v3.color.b, t);

	v1.texture.u = Interpolate(v2.texture.u, v3.texture.u, t);
	v1.texture.v = Interpolate(v2.texture.v, v3.texture.v, t);
}

void VertexDivision(Vertex& v1, const Vertex& v2, const Vertex& v3, float w)
{
	Vector3Division(v1.pos, v2.pos, v3.pos, w);

	float inv = 1.0f / w;
	v1.color.a = (v3.color.a - v2.color.a) * inv;
	v1.color.r = (v3.color.r - v2.color.r) * inv;
	v1.color.g = (v3.color.g - v2.color.g) * inv;
	v1.color.b = (v3.color.b - v2.color.b) * inv;

	v1.texture.u = (v3.texture.u - v2.texture.u) * inv;
	v1.texture.v = (v3.texture.v - v2.texture.v) * inv;
}

void VertexAdd(Vertex& v1, Vertex& v2)
{
	v1.pos.x += v2.pos.x;
	v1.pos.y += v2.pos.y;
	v1.pos.z += v2.pos.z;

	v1.color.a += v2.color.a;
	v1.color.r += v2.color.r;
	v1.color.g += v2.color.g;
	v1.color.b += v2.color.b;

	v1.texture.u += v2.texture.u;
	v1.texture.v += v2.texture.v;
}

void MatrixSetIndentity(Matrix44& mat)
{
	mat.m[0][1] = mat.m[0][2] = mat.m[0][3] = 0.0f;
	mat.m[1][0] = mat.m[1][2] = mat.m[1][3] = 0.0f;
	mat.m[2][0] = mat.m[2][1] = mat.m[2][3] = 0.0f;
	mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0.0f;
	mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;
}

// m = a * b.
void MatrixMulRight( Matrix44& m, Matrix44& a, Matrix44& b )
{
	for (int i = 0; i < 4; i ++)
		for (int j = 0; j < 4; j ++)
		{
			m.m[i][j] = 0.0f;
			for (int k = 0; k < 4; k ++)
				m.m[i][j] += a.m[i][k] * b.m[k][j];
		}
}

//void MatrixSetRotation(Matrix44& m, const Vector3& n, float r)
void MatrixSetRotation(Matrix44& m, const Vector3& n, float r)
{
	float xy = n.x * n.y;
	float yz = n.y * n.z;
	float xz = n.x * n.z;
	float rsin = sin(r);
	float rcos = cos(r);
	float rcost = 1 - rcos;
	float nxsin = n.x * rsin;
	float nysin = n.y * rsin;
	float nzsin = n.z * rsin;
	m.m[0][0] = n.x * n.x * rcost + rcos;
	m.m[0][1] = xy * rcost + nzsin;
	m.m[0][2] = xz * rcost - nysin;

	m.m[1][0] = xy * rcost - nzsin;
	m.m[1][1] = n.y * n.y * rcost + rcos;
	m.m[1][2] = yz * rcost + nxsin;

	m.m[2][0] = xz * rcost + nysin;
	m.m[2][1] = yz * rcost - nxsin;
	m.m[2][2] = n.z * n.z * rcost + rcos;

	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
	m.m[3][3] = 1.0f;
}

void MatrixSetProjection(Matrix44& m, float fov, float aspect, float nz, float zf)
{
	float zoomy = 1.0f / (float) tan(fov * 0.5);
	float zl = 1.0f / (zf - nz);
	MatrixSetIndentity(m);
	m.m[0][0] = zoomy / aspect;
	m.m[1][1] = zoomy;
	m.m[2][2] = zf * zl;
	m.m[3][2] = -nz * zf * zl;
	m.m[2][3] = 1.0f;
	m.m[3][3] = 0.0f;
}

void MatrixSetCamear(Matrix44& mat, const Vector3& eye, const Vector3& look, const Vector3& up)
{
	Vector3 zaxis;
	Vector3Sub(zaxis, look, eye);
	Vector3Normalize(zaxis);
	Vector3 xaxis;
	Vector3Cross(xaxis, up, zaxis);
	Vector3Normalize(xaxis);
	Vector3 yaxis;
	Vector3Cross(yaxis, zaxis, xaxis);

	float x, y, z;
	x = -Vector3Dot(eye, xaxis);
	y = -Vector3Dot(eye, yaxis);
	z = -Vector3Dot(eye, zaxis);

	mat.m[0][0] = xaxis.x;	mat.m[0][1] = yaxis.x;	mat.m[0][2] = zaxis.x;	mat.m[0][3] = 0.0f;
	mat.m[1][0] = xaxis.y;	mat.m[1][1] = yaxis.y;	mat.m[1][2] = zaxis.y;	mat.m[1][3] = 0.0f;
	mat.m[2][0] = xaxis.z;	mat.m[2][1] = yaxis.z;	mat.m[2][2] = zaxis.z;	mat.m[2][3] = 0.0f;
	mat.m[3][0] = x;		mat.m[3][1] = y;		mat.m[3][2] = z;		mat.m[3][3] = 1.0f;
}

void MatrixApply(Vector4& v1, const Vector3& v2, const Matrix44& m)
{
	float x, y, z;
	x = v2.x;
	y = v2.y;
	z = v2.z;
	float w = 1.0f;
	v1.x = x * m.m[0][0] + y * m.m[1][0] + z * m.m[2][0] + w * m.m[3][0];
	v1.y = x * m.m[0][1] + y * m.m[1][1] + z * m.m[2][1] + w * m.m[3][1];
	v1.z = x * m.m[0][2] + y * m.m[1][2] + z * m.m[2][2] + w * m.m[3][2];
	v1.w = x * m.m[0][3] + y * m.m[1][3] + z * m.m[2][3] + w * m.m[3][3];
}

void DeviceClear(RenderDevice* rd)
{
	for (int y = 0; y < rd->mHeight; y ++)
	{
		for ( int x = 0; x < rd->mWidth; x ++ )
		{
			rd->mFrameBuffer[y][x] = 0xff000000;
			rd->mZBuffer[y][x] = 0.0f;
		}
	}
}

bool OnKeyDown(uint key)
{
	static float theta = 0.0f;
	static float phi = 0.0f;
	if (key == 32)
	{
		gRenderDevice.mRenderMethod = (gRenderDevice.mRenderMethod << 1) % 0x07;
	}
	else if (key == 'W')
	{
		gRenderDevice.eye.z += 0.1f;
	}
	else if (key == 'S')
	{
		gRenderDevice.eye.z -= 0.1f;
	}
	else if (key == 'A')
	{
		theta += 0.1f;
	}
	else if (key == 'D')
	{
		theta -= 0.1f;
	}
	else if (key == 'Q')
	{
		phi += 0.1f;
	}
	else if (key == 'E')
	{
		phi -= 0.1f;
	}
	else if (key == 'J')
	{
		gMaterial.ambient.r += 0.1f;
		if (gMaterial.ambient.r > 1.0f)
			gMaterial.ambient.r = 1.0f;
	}
	else if (key == 'K')
	{
		gMaterial.ambient.r -= 0.1f;
		if (gMaterial.ambient.r < 0.0f)
			gMaterial.ambient.r = 0.0f;
	}

	if (key == 'W' || key == 'S')
		MatrixSetCamear(gRenderDevice.mView, gRenderDevice.eye, gRenderDevice.look, gRenderDevice.up);

	if (key == 'A' || key == 'D')
		MatrixSetRotation( gRenderDevice .mWorld, gRenderDevice.up, theta);

	if (key == 'Q' || key == 'E')
	{
		const Vector3 n = {1.0f, 0.0f, 0.0f};
		MatrixSetRotation( gRenderDevice.mWorld, n, phi);
	}
	
	return true;
}

void UpdateTransform(RenderDevice* rd)
{
	Matrix44 m;
	MatrixMulRight(m, rd->mWorld, rd->mView);
	MatrixMulRight(rd->mTransform, m, rd->mProject);
}

void DrawCube(RenderDevice* rd)
{

	for (int i = 50; i < 100; i ++)
		for (int j = 50; j < 100; j ++)
	{
		int A = 255;
		int R = 359;
		int G = 307;
		int B = 287;
		DrawPixel(rd, i, j, (A<<24) | (R<<16) | (G<<8) | B);
	}

		for (int i = 100; i < 155; i ++)
			for (int j = 100; j < 155; j ++)
			{
				int A = 255;
				int R = 256;
				int G = 255;
				int B = 255;
				DrawPixel(rd, i, j, (A<<24) | (R<<16) | (G<<8) | B);
			}

	DrawPlane(rd, 0, 1, 2, 3);
	DrawPlane(rd, 4, 5, 6, 7);
	DrawPlane(rd, 0, 4, 7, 1);
	DrawPlane(rd, 1, 7, 6, 2);
	DrawPlane(rd, 2, 6, 5, 3);
	DrawPlane(rd, 3, 5, 4, 0);
}

void DrawPlane(RenderDevice* rd, int a, int b, int c, int d)
{
	Vertex& v1 = gMesh[a];
	Vertex& v2 = gMesh[b];
	Vertex& v3 = gMesh[c];
	Vertex& v4 = gMesh[d];
	// Need to reset uv.
	v1.texture.u = 0.0f;	v1.texture.v = 0.0f;
	v2.texture.u = 0.0f;	v2.texture.v = 1.0f;
	v3.texture.u = 1.0f;	v3.texture.v = 1.0f;
	v4.texture.u = 1.0f;	v4.texture.v = 0.0f;

	DrawPrimitive(rd, v1, v2, v3);
	DrawPrimitive(rd, v3, v4, v1);
}

bool CheckInCVV(const Vector4& v)
{
	float w = v.w;
	if ( v.x < -w || v.x > w ||
		v.y < -w || v.y > w ||
		v.z < 0 || v.z > w)
		return false;

	return true;
}

void VertexInitByW(Vertex& v, float w)
{
	v.pos.x *= w;
	v.pos.y *= w;
	v.pos.z *= w;

	v.color.a *= w;
	v.color.r *= w;
	v.color.g *= w;
	v.color.b *= w;

	v.texture.u *= w;
	v.texture.v *= w;
}

void DrawPrimitive(RenderDevice* rd, const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	Vector4 c1, c2, c3, p1, p2, p3;
	MatrixApply(c1, v1.pos, rd->mTransform);
	MatrixApply(c2, v2.pos, rd->mTransform);
	MatrixApply(c3, v3.pos, rd->mTransform);

	// 裁剪。
	if (!CheckInCVV(c1) || !CheckInCVV(c2) || !CheckInCVV(c3))
		return;

	// 进行背面消除。计算面法线。
	Vector4 _w1, _w2, _w3;
	MatrixApply(_w1, v1.pos, rd->mWorld);
	MatrixApply(_w2, v2.pos, rd->mWorld);
	MatrixApply(_w3, v3.pos, rd->mWorld);
	
	Vector3 w1, w2, w3;
	GetVector3InVector4(w1, _w1);
	GetVector3InVector4(w2, _w2);
	GetVector3InVector4(w3, _w3);

	Vector3 temp1, temp2;
	Vector3Sub(temp1, w2, w1);
	Vector3Sub(temp2, w3, w1);

	Vector3 normal;
	Vector3Cross(normal, temp1, temp2);
	Vector3Normalize(normal);

	Vector3 view;
	Vector3Sub(view, rd->eye, w1);

	if (Vector3Dot(normal, view) <= 0)
		return;

	ViewPort(rd, p1, c1);
	ViewPort(rd, p2, c2);
	ViewPort(rd, p3, c3);

	if (rd->mRenderMethod & RenderMethod::wireFrame)
	{
		int x1 = (int) p1.x, y1 =(int) p1.y, x2 = (int) p2.x, y2 = (int) p2.y, x3 = (int) p3.x, y3 = (int) p3.y;
		DrawLine(rd, (int) p1.x, (int) p1.y, (int) p2.x, (int) p2.y, 0xff00ff00);
  		DrawLine(rd, (int) p2.x, (int) p2.y, (int) p3.x, (int) p3.y, 0xff00ff00);
  		DrawLine(rd, (int) p3.x, (int) p3.y, (int) p1.x, (int) p1.y, 0xff00ff00);
	}

	if(rd->mRenderMethod & RenderMethod::color || rd->mRenderMethod & RenderMethod::texture)
	{
		Vertex sv1 = v1, sv2 = v2, sv3 = v3;

		// 记录世界坐标，用来插值计算光照。
		sv1.pos = w1;
		sv2.pos = w2;
		sv3.pos = w3;

		p1.w = 1.0f / c1.w;
		p2.w = 1.0f / c2.w;
		p3.w = 1.0f / c3.w;

		VertexInitByW(sv1, p1.w);
		VertexInitByW(sv2, p2.w);
		VertexInitByW(sv3, p3.w);

		int ymin = (int) (MIN(p1.y, p2.y, p3.y) + 0.5f);
		int ymax = (int) (MAX(p1.y, p2.y, p3.y) + 0.5f);

		Edge* edges = (Edge*) malloc(sizeof(Edge) * (ymax - ymin + 1));

		for (int i = 0; i < ymax - ymin + 1; i ++)
			edges[i].next = NULL;

		// Init Net.
		InitNETs(p1, sv1, p2, sv2, p3, sv3, edges, ymin);

		Edge* aet = (Edge*) malloc(sizeof(Edge));
		aet->next = NULL;
		int scanline = ymin;
		for (int i = 0; i < ymax - ymin; i ++) 
		{
			// If net is not null, add to actedge.
			// Build AET.
		
			BuildAET(aet, &edges[i]);

			// DrawScanline.
			DrawScanline(&gRenderDevice, aet, scanline, normal);
			scanline ++;
			// Update edge.
			UpdateAET(aet, scanline);
			// Resort.
			ResortAET(aet);
		}

		// free scanline.
		{
			free(edges);

			Edge *p, *q = aet;
			while (q != NULL)
			{
				p = q->next;
				free(q);
				q = p;
			}
		}
	}
}

void ViewPort(RenderDevice* rd, Vector4& v1, const Vector4& v2)
{
	float invw = 1 / v2.w;
	v1.x = (v2.x * invw + 1.0f) * rd->mWidth * 0.5f;
	v1.y = (1.0f - v2.y * invw) * rd->mHeight * 0.5f;
	v1.z = v2.z * invw;
	v1.w = 1.0f;
}

void InitNETs(Vector4& p1, const Vertex& v1, Vector4& p2, const Vertex& v2, Vector4& p3, const Vertex& v3, Edge* edges, int ymin)
{
	InitNET(p1, v1, p2, v2, edges, ymin);
	InitNET(p2, v2, p3, v3, edges, ymin);
	InitNET(p3, v3, p1, v1, edges, ymin);
}

void InitNET(Vector4&p, const Vertex& v, Vector4& pn, const Vertex& vn, Edge* edges, int minscanline)
{
	int px = (int) (p.x + 0.5f), pnx = (int) (pn.x + 0.5f);
	int py = (int) (p.y + 0.5f), pny = (int) (pn.y + 0.5f);

	if ( py == pny ) return;

	Edge* edge = (Edge*) malloc(sizeof(Edge));

	float height = (float) (pny - py);
	float inv = 1.0f / height;
	edge->dw = (pn.w - p.w) * inv;
	edge->dx = (float) (pnx- px) * inv;

	VertexDivision(edge->dv, v, vn, height);

	if (py < pny)
	{
		edge->x = (float) px;
		edge->ymax = pny;
		edge->w = p.w;
		edge->v = v;

		InsertEdge(&edges[py-minscanline], edge);
	}
	else
	{
		edge->x = (float) pnx;
		edge->ymax = py;
		edge->w = pn.w;
		edge->v = vn;

		InsertEdge(&edges[pny-minscanline], edge);
	}
}