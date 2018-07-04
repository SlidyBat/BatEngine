#include "D3DClass.h"
#include <vector>
#include <algorithm>

ID3D11Device* D3DClass::GetDevice() const
{
	return m_pDevice.Get();
}

ID3D11DeviceContext* D3DClass::GetDeviceContext() const
{
	return m_pDeviceContext.Get();
}

void D3DClass::GetVideoCardInfo( std::wstring& cardName, int& memory ) const
{
	cardName = m_szVideoCardDescription;
	memory = m_iVideoCardMemory;
}

D3DClass::D3DClass( Window& wnd, bool vsyncEnabled, float screendepth, float screennear )
{
	m_bVSyncEnabled = vsyncEnabled;

	Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	if( FAILED( CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**)&factory ) ) )
	{
		throw std::runtime_error( "Failed to create DXGI Factory" );
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	if( FAILED( factory->EnumAdapters( 0, &adapter ) ) )
	{
		throw std::runtime_error( "Failed to enumerate adapters" );
	}

	Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
	if( FAILED( adapter->EnumOutputs( 0, &adapterOutput ) ) )
	{
		throw std::runtime_error( "Failed to enumerate adapter outputs" );
	}

	UINT numModes;
	if( FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL ) ) )
	{
		throw std::runtime_error( "Failed to create DXGI Factory" );
	}

	DXGI_MODE_DESC* displayModes = new DXGI_MODE_DESC[numModes];
	if( !displayModes )
	{
		throw std::runtime_error( "Failed to create display modes array" );
	}

	if( FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModes ) ) )
	{
		throw std::runtime_error( "Failed to get display mode list count" );
	}

	UINT numerator = 0, denominator = 1;
	for( UINT i = 0; i < numModes; i++ )
	{
		if( displayModes[i].Width == (UINT)wnd.GetWidth() && displayModes[i].Height == (UINT)wnd.GetHeight() )
		{
			numerator = displayModes[i].RefreshRate.Numerator;
			denominator = displayModes[i].RefreshRate.Denominator;
		}
	}

	delete[] displayModes;

	DXGI_ADAPTER_DESC adapterDesc;
	if( FAILED( adapter->GetDesc( &adapterDesc ) ) )
	{
		throw std::runtime_error( "Failed to get get adapter description" );
	}

	m_szVideoCardDescription = adapterDesc.Description;
	m_iVideoCardMemory = (int)( adapterDesc.DedicatedVideoMemory / 1024 / 1024 ); // in mb


	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = wnd.GetWidth();
	swapChainDesc.BufferDesc.Height = wnd.GetHeight();
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
	swapChainDesc.OutputWindow = wnd.GetHandle();

	// multisampling
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = !wnd.IsFullscreen();

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.Flags = wnd.IsFullscreen() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

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
		throw std::runtime_error( "Failed to create device/swapchain" );
	}

	ID3D11Texture2D* pBackBuffer;
	if( FAILED( m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
	{
		throw std::runtime_error( "Failed to get swapchain backbuffer" );
	}

	if( FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView ) ) )
	{
		throw std::runtime_error( "Failed to create render target view" );
	}

	pBackBuffer->Release();
	pBackBuffer = nullptr;

	m_pDeviceContext->OMSetRenderTargets( 1, m_pRenderTargetView.GetAddressOf(), NULL );

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
		throw std::runtime_error( "Failed to create rasterizer state" );
	}

	m_pDeviceContext->RSSetState( m_pRasterState.Get() );

	// set up viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)wnd.GetWidth();
	viewport.Height = (float)wnd.GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_pDeviceContext->RSSetViewports( 1, &viewport );
}

void D3DClass::BeginScene( float red, float green, float blue, float alpha )
{
	const float colour[4] = { red, green, blue, alpha };

	m_pDeviceContext->ClearRenderTargetView( m_pRenderTargetView.Get(), colour );
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

void D3DClass::Resize( int width, int height )
{
	m_pDeviceContext->OMSetRenderTargets( 0, NULL, NULL );

	m_pRenderTargetView.Reset();

	HRESULT hr;
	if( FAILED( hr = m_pSwapChain->ResizeBuffers( 0, width, height, DXGI_FORMAT_UNKNOWN, 0 ) ) )
	{
		throw std::runtime_error( "Failed to resize swapchain buffer" );
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBuffer;
	if( FAILED( m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBuffer ) ) )
	{
		throw std::runtime_error( "Failed to get swapchain buffer" );
	}

	if( FAILED( m_pDevice->CreateRenderTargetView( pBuffer.Get(), NULL, &m_pRenderTargetView ) ) )
	{
		throw std::runtime_error( "Failed to create render target view while resizing" );
	}

	m_pDeviceContext->OMSetRenderTargets( 1, m_pRenderTargetView.GetAddressOf(), NULL );

	if( width == 0 && height == 0 )
	{
		DXGI_SWAP_CHAIN_DESC desc;
		m_pSwapChain->GetDesc( &desc );

		HWND hWnd = desc.OutputWindow;

		RECT rect;
		GetClientRect( hWnd, &rect );

		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_pDeviceContext->RSSetViewports( 1, &viewport );
}
