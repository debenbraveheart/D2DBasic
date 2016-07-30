#include "stdafx.h"
#include "VSyncWin.h"

#pragma comment (lib, "D3D11.lib")
#pragma comment (lib, "D2D1.lib" )
VSyncWin* VSyncWin::Self;

VSyncWin::VSyncWin() :
mBlitHandled(true),
mBlittingThreadRun(true),
mBlit(false),
mVSyncTimeStamp(0)
{
	mCurrentBitmap = 1;
	mLastUpdatedBitmap = 2;
	mLastCurrentBitmap = 3;

	mLastBlittedTimeStamp = Now();
}


VSyncWin::~VSyncWin()
{
}

TimeStamp VSyncWin::Now()
{
	LARGE_INTEGER l;
	::QueryPerformanceCounter(&l);
	return l.QuadPart;
	//return GetTickCount64();
}


TimeStamp gTS;
LARGE_INTEGER gS, gE, gSS;

TimeStamp VSyncWin::GetAdjustedVsyncTimeStamp(LARGE_INTEGER& aFrequency,
	QPC_TIME& aQpcVblankTime)
{
	char buf[1024];
	TimeStamp vsync = Now();
	LARGE_INTEGER qpcNow;
	QueryPerformanceCounter(&qpcNow);

	const int microseconds = 1000000;
	int64_t adjust = qpcNow.QuadPart - aQpcVblankTime;
	int64_t usAdjust = (adjust * microseconds) / aFrequency.QuadPart;

	sprintf_s(buf, "\n(((((   usadjust %lld and", usAdjust);
	OutputDebugStringA(buf);
	sprintf_s(buf, "\n(((((Vsync %llu", aQpcVblankTime);
	OutputDebugStringA(buf);
	vsync -= FromMicroseconds((double)usAdjust);


	//if (IsWin10OrLater()) 
	{
		// On Windows 10 and on, DWMGetCompositionTimingInfo, mostly
		// reports the upcoming vsync time, which is in the future.
		// It can also sometimes report a vblank time in the past.
		// Since large parts of Gecko assume TimeStamps can't be in future,
		// use the previous vsync.

		// Windows 10 and Intel HD vsync timestamps are messy and
		// all over the place once in a while. Most of the time,
		// it reports the upcoming vsync. Sometimes, that upcoming
		// vsync is in the past. Sometimes that upcoming vsync is before
		// the previously seen vsync. Sometimes, the previous vsync
		// is still in the future. In these error cases,
		// we try to normalize to Now().
		TimeStamp upcomingVsync = vsync;
		if (upcomingVsync < mPrevVsync) {
			// Windows can report a vsync that's before
			// the previous one. So update it to sometime in the future.
			upcomingVsync = Now() + FromMilliseconds(1);
		}

		
		vsync = mPrevVsync;
		sprintf_s(buf, "\n))))   vsync %llu---", vsync);
		OutputDebugStringA(buf);
		mPrevVsync = upcomingVsync;
	}
	// On Windows 7 and 8, DwmFlush wakes up AFTER qpcVBlankTime
	// from DWMGetCompositionTimingInfo. We can return the adjusted vsync.

	// Once in a while, the reported vsync timestamp can be in the future.
	// Normalize the reported timestamp to now.
	if (vsync >= Now()) {
		vsync = Now();
	}
	return vsync;
}


void  VSyncWin::VSyncLoop()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	DWM_TIMING_INFO vblankTime;
	// Make sure to init the cbSize, otherwise GetCompositionTiming will fail
	vblankTime.cbSize = sizeof(DWM_TIMING_INFO);

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	TimeStamp vsync = Now();
	// On Windows 10, DwmGetCompositionInfo returns the upcoming vsync.
	// See GetAdjustedVsyncTimestamp.
	// On start, set mPrevVsync to the "next" vsync
	// So we'll use this timestamp on the 2nd loop iteration.
	mPrevVsync = vsync + mSoftwareVsyncRate;

	for (;;) {

#if !USING_DWM_FLUSH
		TimeStamp here = Now();

		::QueryPerformanceCounter(&gSS);
		//Sleep(1);
		//Sleep(200);

		
		m_pOutput->WaitForVBlank();

		
		//Sleep(1);
		
		m_pSwapChain->Present(0, 0);
		//DwmFlush();

		//Gdi: NtGdiDdDDIWaitForVerticalBlankEvent()
		//TimerBlit();
		//gS = gSS;
		//::PostMessageA(gHwnd, WM_BLIT_REQUEST, 0, 0);
		//TimerBlit();
		FILE *f;
		fopen_s(&f, "C:\\VSyncThread", "rb");
		//char buf[200];
		LARGE_INTEGER fi;
		char buf[200];
		::QueryPerformanceFrequency(&fi);
		sprintf_s(buf, "\n+++++ %d", int((Now() - here) * 1000000 / fi.QuadPart));
		OutputDebugStringA(buf);
#else
		::QueryPerformanceCounter(&gS);

		gTS = vsync;


		/*mWork = CreateThreadpoolWork(BlittingThread,
		(void*)&gTS,
		&mCallBackEnviron);

		SubmitThreadpoolWork(mWork);*/

		//mVSyncTimeStamp.store(vsync);
		
		char buf[1000];
		LARGE_INTEGER fi;
		//::QueryPerformanceFrequency(&fi);
		//sprintf_s(buf, "\n+++++%d", int((vsync - here) * 1000000 / fi.QuadPart));
		//OutputDebugStringA(buf);

		// Use a combination of DwmFlush + DwmGetCompositionTimingInfoPtr
		// Using WaitForVBlank, the whole system dies :/
		//m_pSwapChain->Present(0, 0);
		//::PostMessageA(gHwnd, WM_BLIT_REQUEST, 0, 0);
		//m_pSwapChain->Present(1, 0);
		//TimerBlit();
		
		HRESULT hr;
		
		hr = DwmFlush();//WinUtils::dwmFlushProcPtr();
		if (!SUCCEEDED(hr)) {
		//	// We don't actually know how long we had to wait on DWMFlush
		//	// Instead of trying to calculate how long DwmFlush actually took
		//	// Fallback to software vsync.
		//	//-->ScheduleSoftwareVsync(Now());

			return;
		}

		//dwmflush does not exactly return at next vblank 
		//TimerBlit();

		hr = DwmGetCompositionTimingInfo(0, &vblankTime);


		/*vsync = SUCCEEDED(hr) ?
		GetAdjustedVsyncTimeStamp(frequency, vblankTime.qpcVBlank) :
		Now();*/
		vsync = vblankTime.qpcVBlank;// +vblankTime.qpcRefreshPeriod;
		mVSyncTimeStamp.store(vsync);
		mBlit = true;
		////char buf[200];
		//LARGE_INTEGER l;
		//

		//QueryPerformanceFrequency(&l);
		////if (((vsync - Now()) * 1000000 / l.QuadPart) > 10000)
		//	//continue;
		///*LARGE_INTEGER le, lf;
		//QueryPerformanceCounter(&le);
		//QueryPerformanceFrequency(&lf);*/
		//sprintf_s(buf, "\nvblank Time  Now =  %u", (vsync - Now()) * 1000000 / l.QuadPart);
		//OutputDebugStringA(buf);
		//sprintf_s(buf, "\n****** ---- %llu --", vsync);
		//OutputDebugStringA(buf);
		/*sprintf_s(buf, "\nv Timed %d ", ((le.QuadPart - gS.QuadPart) * 1000000 / lf.QuadPart));
		OutputDebugStringA(buf);*/
#endif
	} // end for

}


void VSyncWin::VSyncInit()
{
	Self = this;

	CoInitialize(NULL);

	HRESULT hr;
	//hr = CreateDXGIFactory2(__uuidof(IDXGIFactory1), (void**)(&pFactory));

	//if (FAILED(hr))
	//{
	//	exit(0);
	//}
	//UINT i = 0;
	////IDXGIAdapter1*> vAdapters;

	//while (pFactory->EnumAdapters1(i, &m_pAdapter) != DXGI_ERROR_NOT_FOUND)
	//{
	//	//vAdapters.push_back(pAdapter);
	//	//pAdapter->EnumOutputs()
	//	++i;
	//	break; //we want the first adapter
	//}

	//if (m_pAdapter->EnumOutputs(0, &m_pOutput) == DXGI_ERROR_NOT_FOUND)
	//{
	//	//error
	//	exit(0);
	//}

	RECT rc;
	GetClientRect(gHwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// for d2d support.. make it 0 not work
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;


	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		mDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, mDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pD3dDevice, &mFeatureLevel, &m_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, mDriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pD3dDevice, &mFeatureLevel, &m_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		exit(0);
	
	
	const double rate = 1000 / 60.0;
	mSoftwareVsyncRate = FromMilliseconds(rate);
	mPrevVsync = Now();

	/*hr = pFactory->CreateSwapChain(m_pD3dDevice, &scd, &m_pSwapChain);

	if ( FAILED(hr) )
		exit(0);
*/
	
	hr = m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice2), (void **)&m_pDXGIDevice);

	if (FAILED(hr))
	{
		exit(0);
	}

	IDXGIAdapter * pDXGIAdapter;
	hr = m_pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

	if (FAILED(hr))
	{
		exit(0);
	}

	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&	pFactory);


	if (FAILED(hr))
	{
		exit(0);
	}


	hr = pFactory->EnumAdapters1(0, &m_pAdapter);
	if (FAILED(hr))
		exit(0);

	hr = m_pAdapter->EnumOutputs(0, &m_pOutput);
	if (FAILED(hr))
		exit(0);

	DXGI_OUTPUT_DESC desc;
	hr = m_pOutput->GetDesc(&desc);
	if (FAILED(hr))
		exit(0);

	RECT r;
	GetClientRect(gHwnd, &r);

	gHeight = r.bottom - r.top;
	gWidth = r.right - r.left;
	DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
	//scd.BufferDesc.Width = gWidth;
	//scd.BufferDesc.Height = gHeight;
	//scd.BufferDesc.RefreshRate.Numerator = 60;
	//scd.BufferDesc.RefreshRate.Denominator = 1;
	//scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//scd.SampleDesc.Count = 1;
	//scd.SampleDesc.Quality = 0;

	//scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//scd.BufferCount = 3; //1
	//scd.OutputWindow = gHwnd;
	//scd.Windowed = true;
	//scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;// DXGI_SWAP_EFFECT_SEQUENTIAL;//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	//scd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;    // how the swap chain should be used
	scd.BufferCount = 3;                                  // a front buffer and a back buffer
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;              // the most common swap chain format
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;    // the recommended flip mode
	scd.SampleDesc.Count = 1;
	scd.Scaling = DXGI_SCALING_NONE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	scd.Width = gWidth;
	scd.Height = gHeight;
	hr = pFactory->CreateSwapChainForHwnd(m_pD3dDevice, gHwnd, &scd, NULL, NULL, &m_pSwapChain);

	if (FAILED(hr))
	{
		exit(0);
	}

	hr = m_pSwapChain->GetBuffer(0, __uuidof(m_pBackBuffer), reinterpret_cast<void**>(&m_pBackBuffer));
	if (FAILED(hr))
		exit(0);

	hr = m_pD3dDevice->CreateRenderTargetView(m_pBackBuffer, NULL, &m_pRenderTarget ); 
	if (FAILED(hr))
		exit(0);

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTarget, NULL);

	DXGI_MODE_DESC requestedMode;
	requestedMode.Width = gWidth;
	requestedMode.Height = gHeight;
	requestedMode.RefreshRate.Numerator = 0;
	requestedMode.RefreshRate.Denominator = 0;
	requestedMode.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	requestedMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	requestedMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_MODE_DESC mode;
	hr = m_pOutput->FindClosestMatchingMode(&requestedMode, &mode, m_pD3dDevice);
	if (FAILED(hr))
		throw - 1;
	
	hr = m_pSwapChain->ResizeTarget(&mode);
	if (FAILED(hr))
		throw -1; 


	mode.RefreshRate.Numerator = 0; 
	mode.RefreshRate.Denominator = 0;
	hr = m_pSwapChain->ResizeTarget(&mode);
	if (FAILED(hr))
		throw - 1;


	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&m_pD2D1Factory);

	if (FAILED(hr))
	{
		exit(0);
	}

	hr = m_pD2D1Factory->CreateDevice(m_pDXGIDevice, &m_pD2D1Device);
	if (FAILED(hr))
	{
		exit(0);
	}

	
	hr = m_pD2D1Device->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m_pD2DContext
		);
	
	if (FAILED(hr))
	{
		exit(0);
	}
	
	

	// Direct2D needs the dxgi version of the backbuffer surface pointer.

	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_pDxgiBackBuffer));

	if (FAILED(hr))
		exit(0);

	auto d2dRTProps = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 96.0, 96.0);


	/*D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
		96.0,
		96.0
		);*/


	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	/*hr = m_pD2DContext->CreateBitmapFromDxgiSurface(
		m_pDxgiBackBuffer,
		d2dRTProps,
		&m_pD2DTargetBitmap
		);
	if (FAILED(hr))
		exit(0);*/

	hr = m_pD2D1Factory->CreateDxgiSurfaceRenderTarget(m_pDxgiBackBuffer, &d2dRTProps, &m_pD2DRenderTarget);
	
	if (FAILED(hr))
		exit(0);
	//m_pD2DContext->SetTarget(m_pD2DTargetBitmap);

	//m_pDXGIDevice->SetMaximumFrameLatency(3);

#if 0//VSYNC
	//Creating a thread
	std::thread vsync([this] { this->VSyncLoop(); });
	vsync.detach();
#endif

#if 0//USING_DWM_FLUSH
	std::thread VSyncBlit([this] { this->BlittingThread(); });
	VSyncBlit.detach();
#endif

	HDC hdc = GetDC(gHwnd);

	mDCBitmap1 = CreateCompatibleDC(hdc);
	mDCBitmap2 = CreateCompatibleDC(hdc);
	mDCBitmap3 = CreateCompatibleDC(hdc);

	//HDC hdc = GetDC(hWnd);

	RECT rect;
	GetWindowRect(gHwnd, &rect);

	gWidth = rect.right - rect.left;
	gHeight = rect.bottom - rect.top;
	mBitmap1 = CreateCompatibleBitmap(hdc, gWidth, gHeight);
	mBitmap2 = CreateCompatibleBitmap(hdc, gWidth, gHeight);
	mBitmap3 = CreateCompatibleBitmap(hdc, gWidth, gHeight);

	SelectObject(mDCBitmap1, mBitmap1);
	SelectObject(mDCBitmap2, mBitmap2);
	SelectObject(mDCBitmap3, mBitmap3);

	DeleteDC(hdc);


	InitializeThreadpoolEnvironment(&mCallBackEnviron);

	mPool = CreateThreadpool(NULL);

	if (NULL == mPool) {
		OutputDebugStringA("CreateThreadpool failed. LastError: %u\n");
		exit(0);
	}

	//SetThreadpoolThreadMaximum(mPool, 1);
	//SetThreadpoolCallbackPool(&mCallBackEnviron, mPool);

}


inline int static GetTheCurrentbufferToUse(int currentBitmap, int lastUpdatedBitmap)
{
	if (currentBitmap == 1)
	{
		if (lastUpdatedBitmap == 2)
			return 3;
		else
			return 2;
	}
	else if (currentBitmap == 2)
	{
		if (lastUpdatedBitmap == 3)
			return 1;
		else
			return 3;
	}
	else
	{
		if (lastUpdatedBitmap == 2)
			return 1;
		else
			return 2;
	}
}
int xxx;

int startXxx = 50;
void VSyncWin::BitBltEx(HDC   hdcDest, int   nXDest, int   nYDest, int   nWidth, int   nHeight, HDC   hdcSrc, int   nXSrc, int   nYSrc, DWORD dwRop)
{

	/**Triple Buffering ***/

#if 0

	char buf[200];

	int currentBitmapToUSe;

	if (mCurrentBitmap != mLastUpdatedBitmap)
	{
		currentBitmapToUSe = GetTheCurrentbufferToUse(mCurrentBitmap, mLastUpdatedBitmap);
	}
	else
	{
		currentBitmapToUSe = GetTheCurrentbufferToUse(mCurrentBitmap, mLastCurrentBitmap);
	}

	switch (currentBitmapToUSe)
	{
	case 1:
		if (mBitmapLock1.try_lock())
		{
			//mOldBitmap = static_cast<HBITMAP>(SelectObject(mDCBitmap1, mBitmap1));
			BitBlt(mDCBitmap1, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
			mTimestamp1 = Now();
			//SelectObject(mDCBitmap1, mOldBitmap);
			mBitmapLock1.unlock();
		}
		else
		{
			xxx++;
		}
		mLastUpdatedBitmap = 1;
		break;
	case 2:
		if (mBitmapLock2.try_lock())
		{
			//mOldBitmap = static_cast<HBITMAP>(SelectObject(mDCBitmap2, mBitmap2));
			BitBlt(mDCBitmap2, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
			mTimestamp2 = Now();
			//SelectObject(mDCBitmap2, mOldBitmap);
			mBitmapLock2.unlock();
		}
		else
		{
			xxx++;
		}

		mLastUpdatedBitmap = 2;

		break;
	case 3:
		if (mBitmapLock3.try_lock())
		{
			//mOldBitmap = static_cast<HBITMAP>(SelectObject(mDCBitmap3, mBitmap3));
			BitBlt(mDCBitmap3, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
			mTimestamp3 = Now();
			//SelectObject(mDCBitmap3, mOldBitmap);
			mBitmapLock3.unlock();
		}
		else
		{
			xxx++;
		}
		mLastUpdatedBitmap = 3;
		break;
	}
	//sprintf_s(buf, "\nCurrentBitmapUsedToBuufer %d  %d %lld\n", currentBitmapToUSe, 6 - (currentBitmapToUSe + mCurrentBitmap), Now());
	//sprintf_s(buf, "\mCurrentBitmap.... %d  %d %lld\n", mCurrentBitmap, mLastCurrentBitmap, Now());
	//OutputDebugStringA(buf);
	/*

	//mCurrentBitmapMutex.lock();
	{
	if (mCurrentBitmap.load()) //if mCurrentBitmap = 1 means blit should use mDCBitmap1 (1st buffer)
	{
	//so we are copying to the other buffer (0)
	BitBlt(mDCBitmap1, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);
	//flip
	mCurrentBitmap.store(0);
	}
	else //if mCurrentBitmap = 0 means blit should use mDCBitmap2(2nd buffer)
	{
	BitBlt(mDCBitmap2, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);
	//flip(0)
	mCurrentBitmap.store(1);
	}

	//mCurrentBitmapMutex.unlock();
	}
	*/
#else

	HRESULT hr;

	
	m_pD2DRenderTarget->BeginDraw();

	m_pD2DRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	ID2D1SolidColorBrush  *m_pBlackBrush;
	hr = m_pD2DRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&m_pBlackBrush
		);

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(
		D2D1::Point2F(startXxx, 100.f),
		75.f,
		50.f
		);
	startXxx += 16;
	if (startXxx > gWidth)
		startXxx = 0;
	m_pD2DRenderTarget->DrawEllipse(ellipse, m_pBlackBrush, 30.f);	m_pD2DRenderTarget->EndDraw();

	//IDXGISurface1* pSurface1 = NULL;
	//hr = m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&pSurface1);


	ID3D11RenderTargetView* dv = NULL;
	float color[4] = { 5.0f, .2f, 1.0f, 1.0f };

	//m_pImmediateContext->ClearRenderTargetView(m_pRenderTarget, color);
	
	//HDC hDC;

	//
	//hr = pSurface1->GetDC(FALSE, &hDC);
	//
	////DRAW
	//bool dd = BitBlt(hDC, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);

	////BitBlt(GetDC(gHwnd), nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);

	//ReleaseDC(gHwnd ,hDC);
	//pSurface1->ReleaseDC(NULL);
	//pSurface1->Release();
	FILE *f;
	fopen_s(&f, "C:\\PresentWithSwapChain", "rb");

	LARGE_INTEGER as, ae, af;
	QueryPerformanceFrequency(&af);
	QueryPerformanceCounter(&as);
	
	DXGI_PRESENT_PARAMETERS pp;
	pp.DirtyRectsCount = 0;
	pp.pDirtyRects = NULL;
	pp.pScrollRect = NULL;
	pp.pScrollOffset = NULL;

	//static int i = 0;


	m_pSwapChain->Present(1, 0);
	

	QueryPerformanceCounter(&ae);

	char buf[200];
	ae.QuadPart = ae.QuadPart - as.QuadPart;
	ae.QuadPart *= 1000000;
	ae.QuadPart /= af.QuadPart;
	sprintf_s(buf, "\nval %d", ae.QuadPart);
	OutputDebugStringA(buf);
#endif
}


void VSyncWin::TimerBlit()
{

#if 0
	//mBitmapLock.lock();
	{

		//here we can blit
		char buf[200];
#if VSYNC

		if (mLastUpdatedBitmap == mCurrentBitmap)
			return;
		int temp = mCurrentBitmap;
		mCurrentBitmap.store(mLastUpdatedBitmap);
		mLastCurrentBitmap.store(temp);


		if (mCurrentBitmap == 1)
		{
			//if (mBitmapLock1.try_lock())
			//{

			//	//if (mLastBlittedTimeStamp < mTimestamp1)
			//	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap1, 0, 0, SRCCOPY);
			//	mBitmapLock1.unlock();
			//}
			//else
			//{
			//	xxx++;
			//}
			////mCurrentBitmap.store(0);

			BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap1, 0, 0, SRCCOPY);
		}
		else if (mCurrentBitmap == 2)
		{
			//if (mBitmapLock2.try_lock())
			//{
			//	//if (mLastBlittedTimeStamp < mTimestamp2)
			//	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap2, 0, 0, SRCCOPY);
			//	mBitmapLock2.unlock();
			//}
			//else
			//{
			//	xxx++;
			//}
			////mCurrentBitmap.store(1);

			BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap2, 0, 0, SRCCOPY);
		}
		else
		{
			//if (mBitmapLock3.try_lock())
			//{
			//	//if (mLastBlittedTimeStamp < mTimestamp3)
			//	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap3, 0, 0, SRCCOPY);
			//	mBitmapLock3.unlock();
			//}
			//else{
			//	xxx++;
			//}

			BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap3, 0, 0, SRCCOPY);
		}
		//mLastBlittedTimeStamp = Now();
		/*switch (mCurrentBitmap)
		{
		case 1:
			sprintf_s(buf, "\n^^^^^^^^^^^^^^^^^CurrentBitmap....    %lld\n", mTimestamp1);
			break;
		case 2:
			sprintf_s(buf, "\n^^^^^^^^^^^^^^^^^CurrentBitmap....    %lld\n", mTimestamp2);
			break;
		case 3:
			sprintf_s(buf, "\n^^^^^^^^^^^^^^^^^CurrentBitmap....    %lld\n", mTimestamp3);
			break;
		}
		
		OutputDebugStringA(buf);*/

		::QueryPerformanceCounter(&gE);
		gE.QuadPart -= gSS.QuadPart;
		gE.QuadPart *= 1000000;

		LARGE_INTEGER f;
		::QueryPerformanceFrequency(&f);

		gE.QuadPart /= f.QuadPart;

		//char buf[200];
		sprintf_s(buf, "\n******* %d", gE.QuadPart);
		OutputDebugStringA(buf);
#endif
		//mBitmapLock.unlock();

	}

	//mBlitHandled = true;

#else

	if (mLastUpdatedBitmap == mCurrentBitmap)
		return;


	IDXGISurface1* pSurface1 = NULL;
	HRESULT hr;

	hr = m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface1), (void**)&pSurface1);

	if (FAILED(hr))
		exit(0);

	HDC hDC;

	hr = pSurface1->GetDC(FALSE, &hDC);

	if (FAILED(hr))
	{
		char msg[1024];
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);

		OutputDebugStringA(msg);
	}

	
	int temp = mCurrentBitmap;
	mCurrentBitmap.store(mLastUpdatedBitmap);
	mLastCurrentBitmap.store(temp);


	if (mCurrentBitmap == 1)
	{
		//BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap1, 0, 0, SRCCOPY);
		BitBlt(hDC, 0, 0, gWidth, gHeight, mDCBitmap1, 0, 0, SRCCOPY);
	}
	else if (mCurrentBitmap == 2)
	{
		//BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap2, 0, 0, SRCCOPY);
		BitBlt(hDC, 0, 0, gWidth, gHeight, mDCBitmap2, 0, 0, SRCCOPY);
	}
	else
	{
		//BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap3, 0, 0, SRCCOPY);
		BitBlt(hDC, 0, 0, gWidth, gHeight, mDCBitmap3, 0, 0, SRCCOPY);
	}	

	ReleaseDC(gHwnd, hDC);
	pSurface1->ReleaseDC(NULL);

	LARGE_INTEGER as, ae, af;
	QueryPerformanceFrequency(&af);
	QueryPerformanceCounter(&as);

	//Present

	m_pSwapChain->Present(1, DXGI_PRESENT_RESTART);

	char buf[200];
	QueryPerformanceCounter(&ae);
	ae.QuadPart = ae.QuadPart - as.QuadPart;
	ae.QuadPart *= 1000000;
	ae.QuadPart /= af.QuadPart;

	sprintf_s(buf, "\nval %d\n", ae.QuadPart);
	OutputDebugStringA(buf);

#endif
}

void VSyncWin::TimerBlit(HDC hdc)
{

	FILE *f;
	fopen_s(&f, "c:\\Blit", "rb");
	//mBitmapLock.lock();
	{

		//here we can blit
		char buf[200];
#if VSYNC

		if (mLastUpdatedBitmap == mCurrentBitmap)
			return;
		int temp = mCurrentBitmap;
		mCurrentBitmap.store(mLastUpdatedBitmap);
		mLastCurrentBitmap.store(temp);


		if (mCurrentBitmap == 1)
		{
			//if (mBitmapLock1.try_lock())
			//{

			//	//if (mLastBlittedTimeStamp < mTimestamp1)
			//	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap1, 0, 0, SRCCOPY);
			//	mBitmapLock1.unlock();
			//}
			//else
			//{
			//	xxx++;
			//}
			//mCurrentBitmap.store(0);

			BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap1, 0, 0, SRCCOPY);
		}
		else if (mCurrentBitmap == 2)
		{
			//if (mBitmapLock2.try_lock())
			//{
			//	//if (mLastBlittedTimeStamp < mTimestamp2)
			//	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap2, 0, 0, SRCCOPY);
			//	mBitmapLock2.unlock();
			//}
			//else
			//{
			//	xxx++;
			//}
			//mCurrentBitmap.store(1);

			BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap2, 0, 0, SRCCOPY);
		}
		else
		{
			//if (mBitmapLock3.try_lock())
			//{
			//	//if (mLastBlittedTimeStamp < mTimestamp3)
			//	BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap3, 0, 0, SRCCOPY);
			//	mBitmapLock3.unlock();
			//}
			//else{
			//	xxx++;
			//}

			BitBlt(GetDC(gHwnd), 0, 0, gWidth, gHeight, mDCBitmap3, 0, 0, SRCCOPY);
		}

		mLastBlittedTimeStamp = Now();
		sprintf_s(buf, "\mCurrentBitmap....        %d    %lld\n", mCurrentBitmap, Now());
		//OutputDebugStringA(buf);

		::QueryPerformanceCounter(&gE);
		gE.QuadPart -= gSS.QuadPart;
		gE.QuadPart *= 1000000;

		LARGE_INTEGER f;
		::QueryPerformanceFrequency(&f);

		gE.QuadPart /= f.QuadPart;

		//char buf[200];
		sprintf_s(buf, "\n*** %d", gE.QuadPart);
		OutputDebugStringA(buf);
#endif
		//mBitmapLock.unlock();

	}

	//mBlitHandled = true;
}

void VSyncWin::StaticTimer(LPVOID lpArg,               // Data value
	DWORD dwTimerLowValue,      // Timer low value
	DWORD dwTimerHighValue)
{
	Self->TimerBlit();
	//::PostMessageA(gHwnd, WM_BLIT_REQUEST, 0, 0);
	::QueryPerformanceCounter(&gE);
	gE.QuadPart -= gSS.QuadPart;
	gE.QuadPart *= 1000000;

	LARGE_INTEGER f;
	::QueryPerformanceFrequency(&f);

	gE.QuadPart /= f.QuadPart;

	char buf[200];
	sprintf_s(buf, "\n???????????? %u", (int)gE.QuadPart);
	OutputDebugStringA(buf);
}

HANDLE VSyncWin::mHTimer;
void VSyncWin::BlittingThread(PTP_CALLBACK_INSTANCE Instance,
	PVOID                 Parameter,
	PTP_WORK              Work)
{
	//another thread
	BOOL            bSuccess;
	__int64         qwDueTime;
	LARGE_INTEGER   liDueTime;

	TimeStamp vsync = *((TimeStamp*)(Parameter));
	mHTimer = CreateWaitableTimer(
		NULL,                   // Default security attributes
		FALSE,                  // Create auto-reset timer
		TEXT("MyTimer"));       // Name of waitable timer

	char buf[200];
	LARGE_INTEGER lf;
	QueryPerformanceFrequency(&lf);
	sprintf_s(buf, "\n----- %lu", vsync);//(vsync - Self->Now() ) * 1000000 / lf.QuadPart);
	OutputDebugStringA(buf);
	//liDueTime.QuadPart = vsync;//vsync;
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	TimeStamp here = Self->Now();
	uint64_t t = ((vsync - here) * 10000000L / li.QuadPart);
	t = t* -1;
	liDueTime.LowPart = (DWORD)(t & 0xFFFFFFFF);
	liDueTime.HighPart = (LONG)(t >> 32);
	//liDueTime.QuadPart = (liDueTime.QuadPart | (1LL << 63));
	//QueryPerformanceCounter(&gS);
	if (!SetWaitableTimer(mHTimer, &liDueTime, 0, StaticTimer, NULL, FALSE))
		liDueTime.QuadPart = 0;


	SleepEx(INFINITE,     // Wait forever
		TRUE);

}

static int timer;

#include <timeapi.h>

#pragma comment(lib, "Winmm.lib");

void VSyncWin::BlittingThread()
{
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	::timeBeginPeriod(1);
	HANDLE BogusTime;
	BogusTime = CreateEventA(0, TRUE, FALSE, 0);

	LARGE_INTEGER   liDueTime, lf;
	QueryPerformanceFrequency(&lf);

	while (mBlittingThreadRun)
	{
		
		if (mBlit)
		{
			mBlit = false;

		
			BOOL            bSuccess;
			__int64         qwDueTime;

			::QueryPerformanceCounter(&gSS); // gSS = gS;
			
			TimeStamp vsync = mVSyncTimeStamp.load();

			TimeStamp now = Now();

			if (vsync < now)
			{
				OutputDebugStringA("\ndddddddddddd");
				TimerBlit();
				continue;
			}

#if 0
			if (timer == 0)
			{
				mHTimer = CreateWaitableTimer(
					NULL,                   // Default security attributes
					FALSE,                  // Create auto-reset timer
					TEXT("MyTimer"));       // Name of waitable timer
				timer++;
			}
			char buf[200];
			
			sprintf_s(buf, "\n----- %u ---- %llu", (int)((vsync - Now()) * 1000000 / lf.QuadPart), vsync);
			OutputDebugStringA(buf);

			//liDueTime.QuadPart = vsync*-1;//-140000LL;//(vsync - Now()) * -1 * (1000000 / lf.QuadPart);//-Now(); //-160000LL;//vsync;
			liDueTime.QuadPart = -130000LL;
			/*LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			TimeStamp here = Self->Now();
			uint64_t t = ((vsync - here -5000) * 10000000LL / li.QuadPart);
			t = t* -1;
			liDueTime.LowPart = (DWORD)(t & 0xFFFFFFFF);
			liDueTime.HighPart = (LONG)(t >> 32);*/

			/*
			liDueTime.QuadPart = (liDueTime.QuadPart | (1LL << 63));
			QueryPerformanceCounter(&gS);*/
			

			if (!SetWaitableTimer(mHTimer, &liDueTime, 0, StaticTimer, NULL, FALSE))
				liDueTime.QuadPart = 0;


			SleepEx(INFINITE,     // Wait forever
				TRUE);

			/*SleepEx(6, TRUE);
			TimerBlit();*/
#else
			int diff = ((vsync - now) * 1000 / lf.QuadPart);
			
			::WaitForSingleObject(BogusTime, 10);

			::PostMessageA(gHwnd, WM_BLIT_REQUEST, 0, 0);
			//TimerBlit();
			//m_pSwapChain->Present(1, 0);
			char buf[200];
			sprintf_s(buf, "\n$$$ Time diff %d", diff);
			OutputDebugStringA(buf);

#endif
		}

	}

	::timeEndPeriod(1);
}