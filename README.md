# D2DBasic

# initialization --->
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


----- use

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

	m_pD2DRenderTarget->EndDraw();
