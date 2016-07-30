#pragma once

#include <Windows.h>
#include <dxgi.h>
#include <cstdio>
#include <vector>
#include <d2d1_1.h>
#include <thread>
#include <dwmapi.h>
#include <mutex>
#include <atomic>
#include <d3d11_1.h>
#include <D2d1_1helper.h>

static const double kNsPerMsd = 1000000.0;

extern HWND gHwnd;
extern int gWidth;
extern int gHeight; 

typedef uint64_t TimeStamp;

#define VSYNC 1
#define USING_DWM_FLUSH 1

#define TIMER_VSYNC 3988


#define WM_BLIT_REQUEST WM_USER+3988

class VSyncWin
{
public:
	VSyncWin();
	~VSyncWin();

private:
	TimeStamp mPrevVsync;
	TimeStamp mSoftwareVsyncRate;

	/*  Both the buffer ***/
	HDC mDCBitmap1;
	HBITMAP mBitmap1;
	TimeStamp mTimestamp1;

	HDC mDCBitmap2;
	HBITMAP mBitmap2;
	TimeStamp mTimestamp2;

	HDC mDCBitmap3;
	HBITMAP mBitmap3;
	TimeStamp mTimestamp3;

	HBITMAP mOldBitmap;

	std::atomic<int> mLastUpdatedBitmap;
	std::atomic<int> mCurrentBitmap;
	std::atomic<int> mLastCurrentBitmap;

	std::mutex mCurrentBitmapMutex1;
	std::mutex mCurrentBitmapMutex2;


	TimeStamp mLastBlittedTimeStamp;
	/********************/
	bool mBlitHandled;

	std::mutex mBitmapLock1;
	std::mutex mBitmapLock2;
	std::mutex mBitmapLock3;

	static HANDLE mHTimer;

	PTP_POOL mPool;
	PTP_WORK mWork;
	TP_CALLBACK_ENVIRON mCallBackEnviron;

	static VSyncWin *Self;

	std::atomic<bool> mBlittingThreadRun;
	std::atomic<bool> mBlit;
	volatile std::atomic<TimeStamp> mVSyncTimeStamp;


	IDXGIFactory2 * pFactory;
	IDXGIAdapter1 * m_pAdapter;
	IDXGIOutput *m_pOutput;

	IDXGISwapChain1 *m_pSwapChain;


	D3D_DRIVER_TYPE         mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*           m_pD3dDevice = nullptr;
	ID3D11DeviceContext*    m_pImmediateContext = nullptr;
	IDXGIDevice2*			m_pDXGIDevice;
	ID3D11Resource*			m_pBackBuffer = 0;
	ID3D11RenderTargetView* m_pRenderTarget = 0;

	ID2D1Factory1*	m_pD2D1Factory;
	ID2D1Device*	m_pD2D1Device;
	ID2D1DeviceContext*  m_pD2DContext;
	IDXGISurface* m_pDxgiBackBuffer;
	ID2D1Bitmap1* m_pD2DTargetBitmap;
	ID2D1RenderTarget* m_pD2DRenderTarget;
public:

	static TimeStamp FromMilliseconds(double aMilliseconds)
	{

		double result = aMilliseconds * kNsPerMsd;
		if (result > INT64_MAX) {
			return INT64_MAX;
		}
		else if (result < INT64_MIN) {
			return INT64_MIN;
		}
	}

	static inline TimeStamp FromMicroseconds(double aMicroseconds)
	{
		return FromMilliseconds(aMicroseconds / 1000.0);
	}

	TimeStamp GetAdjustedVsyncTimeStamp(LARGE_INTEGER& aFrequency,
		QPC_TIME& aQpcVblankTime);

	TimeStamp Now();

	void  VSyncLoop();

	void VSyncInit();

	void BitBltEx(HDC   hdcDest,int   nXDest, int   nYDest, int   nWidth, int   nHeight, HDC   hdcSrc, int   nXSrc, int   nYSrc, DWORD dwRop);

	void  TimerBlit();
	void  TimerBlit(HDC hdc);

	static void CALLBACK  StaticTimer(LPVOID lpArg,               // Data value
		DWORD dwTimerLowValue,      // Timer low value
		DWORD dwTimerHighValue);

	static void CALLBACK BlittingThread(PTP_CALLBACK_INSTANCE Instance,
		PVOID                 Parameter,
		PTP_WORK              Work);

	void BlittingThread();
};

