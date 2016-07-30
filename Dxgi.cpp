// Dxgi.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Dxgi.h"
#include "VSyncWin.h"
#include <timeapi.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HDC gSurface;
HBITMAP gBitmap;
HBITMAP gOldBitmap;

int gWidth;
int gHeight;

HWND gHwnd;

void RenderInit();
void Render(HDC surface);

extern int startX;
extern int startY;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DXGI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DXGI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	/*while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, true))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render(gSurface);
		}
	}
*/
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DXGI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DXGI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

VSyncWin *gVsyncWin;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   SetProcessDPIAware();

   gHwnd = hWnd;

   gVsyncWin = new VSyncWin();
   gVsyncWin->VSyncInit();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

  

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
void init();

#define IDT_TIMER 1

#pragma comment(lib, "Winmm.lib")

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	{
					//gWidth = LOWORD(lParam);
					//gHeight = HIWORD(wParam);
					TIMECAPS tc;
					UINT     wTimerRes;

					if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
					{
						// Error; application can't continue.
					}
					else
					{
						char buf[200];
						sprintf_s(buf, "\nTimer %u", tc.wPeriodMin);
						OutputDebugStringA(buf);
					}
					
					HDC hdc = GetDC(hWnd);
					gSurface = CreateCompatibleDC(hdc);

					//HDC hdc = GetDC(hWnd);

					RECT rect;
					GetWindowRect(hWnd, &rect);

					gWidth = rect.right - rect.left;
					gHeight = rect.bottom - rect.top;
					gBitmap = CreateCompatibleBitmap(hdc, gWidth, gHeight);

					gOldBitmap = (HBITMAP)SelectObject(gSurface, gBitmap);
					DeleteDC(hdc);

					SetTimer(hWnd, IDT_TIMER, 10, NULL);


					RenderInit();
					//resize the backbuffer accordingly
					//SelectObject(gSurface, gOldBitmap);


					DeleteDC(hdc);
	}
	case WM_CREATE:
	{
					  BOOL comp;
					  if (DwmIsCompositionEnabled(&comp) != S_OK)
					  {
						  exit(0);
					  }
					  
					  break;
	}

	case WM_KEYDOWN:
	{
					 RECT r = { 0, gHeight - 10,  gWidth, gHeight };
					 //ScrollWindow(gHwnd, 0, -10, NULL, NULL);
					 //startX += 10;
					 startY += 16;
					 //InvalidateRect(hWnd, NULL, FALSE);
					 Render(gSurface);
					 break;
	}
	case WM_BLIT_REQUEST:
		/*hdc = GetDC(hWnd);
		gVsyncWin->TimerBlit(hdc);
		DeleteDC(hdc);*/
		
		//gVsyncWin->TimerBlit();
		
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			//init();
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		//Render(gSurface);
		//Rectangle(hdc, 100, 100, 500, 500);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case IDT_TIMER:
			gOldBitmap = (HBITMAP)SelectObject(gSurface, gBitmap);
			//startY += 16;
			Render(gSurface);
			SelectObject(gSurface, gOldBitmap);
			break;
		case TIMER_VSYNC:
			//gVsyncWin->TimerBlit();
			break;
		}
			
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
