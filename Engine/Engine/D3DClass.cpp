#include "D3DClass.h"

ID3D11Device * D3DClass::GetDevice() const
{
	return m_pDevice;
}

ID3D11DeviceContext * D3DClass::GetDeviceContext() const
{
	return m_pDeviceContext;
}

DirectX::XMMATRIX D3DClass::GetProjectionMatrix() const
{
	return m_matProjectionMatrix;
}

DirectX::XMMATRIX D3DClass::GetWorldMatrix() const
{
	return m_matWorldMatrix;
}

DirectX::XMMATRIX D3DClass::GetOrthoMatrix() const
{
	return m_matOrthoMatrix;
}

void D3DClass::GetVideoCardInfo( std::wstring& cardName, int& memory ) const
{
	cardName = m_szVideoCardDescription;
	memory = m_iVideoCardMemory;
}

D3DClass::~D3DClass()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if( m_pSwapChain )
	{
		m_pSwapChain->SetFullscreenState( false, NULL );
	}

	if( m_pRasterState )
	{
		m_pRasterState->Release();
		m_pRasterState = nullptr;
	}

	if( m_pDepthStencilView )
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

	if( m_pDepthStencilState )
	{
		m_pDepthStencilState->Release();
		m_pDepthStencilState = nullptr;
	}

	if( m_pDepthStencilBuffer )
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;
	}

	if( m_pRenderTargetView )
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}

	if( m_pDeviceContext )
	{
		m_pDeviceContext->Release();
		m_pDeviceContext = nullptr;
	}

	if( m_pDevice )
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}

	if( m_pSwapChain )
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
}

bool D3DClass::Initialize( int screenWidth, int screenHeight, bool fullscreen, HWND hWnd, bool vsyncEnabled, float screendepth, float screennear )
{
	m_bVSyncEnabled = vsyncEnabled;

	IDXGIFactory* factory;
	if( FAILED( CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**)&factory ) ) )
	{
		return false;
	}

	IDXGIAdapter* adapter;
	if( FAILED( factory->EnumAdapters( 0, &adapter ) ) )
	{
		return false;
	}

	IDXGIOutput* adapterOutput;
	if( FAILED( adapter->EnumOutputs( 0, &adapterOutput ) ) )
	{
		return false;
	}

	UINT numModes;
	if( FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL ) ) )
	{
		return false;
	}

	DXGI_MODE_DESC* displayModes = new DXGI_MODE_DESC[numModes];
	if( !displayModes )
	{
		return false;
	}

	if( FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModes ) ) )
	{
		return false;
	}

	UINT numerator = 0, denominator = 1;
	for( UINT i = 0; i < numModes; i++ )
	{
		if( displayModes[i].Width == (UINT)screenWidth && displayModes[i].Height == (UINT)screenHeight )
		{
			numerator = displayModes[i].RefreshRate.Numerator;
			denominator = displayModes[i].RefreshRate.Denominator;
		}
	}

	DXGI_ADAPTER_DESC adapterDesc;
	if( FAILED( adapter->GetDesc( &adapterDesc ) ) )
	{
		return false;
	}


	m_szVideoCardDescription = adapterDesc.Description;
	m_iVideoCardMemory = (int)( adapterDesc.DedicatedVideoMemory / 1024 / 1024 ); // in mb

	delete[] displayModes;
	displayModes = nullptr;

	adapterOutput->Release();
	adapterOutput = nullptr;

	adapter->Release();
	adapter = nullptr;

	factory->Release();
	factory = nullptr;


	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if( m_bVSyncEnabled )
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;

	// multisampling
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = !fullscreen;

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.Flags = 0;

	//D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	if( FAILED( D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		NULL, 0,
		D3D11_SDK_VERSION,
		&swapChainDesc, &m_pSwapChain,
		&m_pDevice, NULL, &m_pDeviceContext ) ) )
	{
		return false;
	}

	ID3D11Texture2D* pBackBuffer;
	if( FAILED( m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
	{
		return false;
	}

	if( FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView ) ) )
	{
		return false;
	}

	pBackBuffer->Release();
	pBackBuffer = nullptr;


	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	if( FAILED( m_pDevice->CreateTexture2D( &depthBufferDesc, NULL, &m_pDepthStencilBuffer ) ) )
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// if pixel front facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// if pixel back facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	if( FAILED( m_pDevice->CreateDepthStencilState( &depthStencilDesc, &m_pDepthStencilState ) ) )
	{
		return false;
	}

	m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilState, 1 );

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if( FAILED( m_pDevice->CreateDepthStencilView( m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView ) ) )
	{
		return false;
	}

	m_pDeviceContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );

	// set up raster description
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	if( FAILED( m_pDevice->CreateRasterizerState( &rasterDesc, &m_pRasterState ) ) )
	{
		return false;
	}

	m_pDeviceContext->RSSetState( m_pRasterState );

	// set up viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_pDeviceContext->RSSetViewports( 1, &viewport );

	// set up projection matrix
	float fieldOfView = 3.141592654f / 4.0f; // pi/4
	float screenAspect = (float)screenWidth / (float)screenHeight;

	m_matProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH( fieldOfView, screenAspect, screennear, screendepth );
	m_matWorldMatrix = DirectX::XMMatrixIdentity();
	m_matOrthoMatrix = DirectX::XMMatrixOrthographicLH( (float)screenWidth, (float)screenHeight, screennear, screendepth );

	return true;
}

void D3DClass::BeginScene( float red, float green, float blue, float alpha )
{
	const float colour[4] = { red, green, blue, alpha };

	m_pDeviceContext->ClearRenderTargetView( m_pRenderTargetView, colour );
	m_pDeviceContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

void D3DClass::EndScene()
{
	if( m_bVSyncEnabled )
	{
		m_pSwapChain->Present( 1, 0 );
	}
	else
	{
		m_pSwapChain->Present( 0, 0 );
	}
}
