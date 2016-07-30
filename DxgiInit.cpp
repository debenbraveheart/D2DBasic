#include "stdafx.h"
#include <Windows.h>
#include <dxgi.h>
#include <cstdio>
#include <vector>
#include <d2d1.h>
#include <thread>
#include <dwmapi.h>
#include "VSyncWin.h"

#pragma comment(lib , "dxgi.lib")
#pragma comment(lib , "Dwmapi.lib")

typedef uint64_t TimeStamp;


extern HWND gHwnd;
extern int gWidth;
extern int gHeight;

#define HEIGHT 50
#define WIDTH 50

int startX = 300;
int startY = 150;

LARGE_INTEGER s, f, e;

HFONT font;
HPEN hpen;

void RenderInit()
{
	s.QuadPart = f.QuadPart = e.QuadPart= 0;

	QueryPerformanceFrequency(&f);

	QueryPerformanceCounter(&s);

	font = ::CreateFont(-140, 0, 0, 0, 200, FALSE, 0, 0, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, L"Consolas");

	hpen = CreatePen(PS_SOLID, 20, RGB(255, 0, 0));
}

extern VSyncWin *gVsyncWin;

int ssss = 0;
void Render(HDC surface)
{
	if (ssss == 1)
		return;

	//ssss++;
	//PAINTSTRUCT ps;
	//BeginPaint(gHwnd, &ps);

	//fill our backbuffer with white
	BitBlt(surface,
		0,
		0,
		gWidth,
		gHeight,
		NULL,
		NULL,
		NULL,
		WHITENESS);

	
	SelectObject(surface, hpen);
	

	if (startX > gWidth)
		startX = 0;
	//Rectangle(surface, startX, startY, startX + WIDTH, startY+HEIGHT);
	RECT rect = { startX, startY, startX + 400, startY + 150 };

	SelectObject(surface, font);

	BitBlt(surface,
	0,
	startY,
	gWidth,
	150,
	NULL,
	NULL,
	NULL,
	WHITENESS);
	DrawTextA(surface, "ADOBE", 6, &rect, 0);
	//TextOutA(surface, startX, startY, "ADOBE", 6);
	startX += 16;

#if VSYNC
	gVsyncWin->BitBltEx(NULL, 0, 0, gWidth, gHeight, surface, 0, 0, SRCCOPY);
#else
	//now blit backbuffer to front
	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, surface, 0, 0, SRCCOPY);
#endif

	//DeleteObject(hpen);
	//DeleteObject(font);

	QueryPerformanceCounter(&e);
	e.QuadPart = e.QuadPart - s.QuadPart;

	e.QuadPart *= 1000;
	e.QuadPart /= f.QuadPart;

	char buf[200];
	sprintf_s(buf, "\ntime went %d", e.QuadPart);
	//OutputDebugStringA(buf);
	QueryPerformanceCounter(&s);
	//EndPaint(gHwnd, &ps);
}
