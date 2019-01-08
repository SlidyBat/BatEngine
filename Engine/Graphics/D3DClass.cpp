#include "PCH.h"
#include "D3DClass.h"

#include <d3d11.h>
#include "COMException.h"
#include "RenderTexture.h"
#include "Common.h"

namespace Bat
{
	static D3D11_VIEWPORT g_Viewport;

	static const char* FeatureLevel2String( D3D_FEATURE_LEVEL );

	D3DClass::D3DClass( Window& wnd, bool vsyncEnabled, float screendepth, float screennear )
	{
		m_bVSyncEnabled = vsyncEnabled;

		Microsoft::WRL::ComPtr<IDXGIFactory> factory;
		COM_THROW_IF_FAILED( CreateDXGIFactory( __uuidof(IDXGIFactory), (void**)&factory ) );

		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> a;
			SIZE_T max_vram = 0;
			for( int i = 0; SUCCEEDED( factory->EnumAdapters( i, &a ) ); i++ )
			{
				DXGI_ADAPTER_DESC adapterDesc;
				COM_THROW_IF_FAILED( a->GetDesc( &adapterDesc ) );

				if( adapterDesc.DedicatedVideoMemory >= max_vram )
				{
					max_vram = adapterDesc.DedicatedVideoMemory;
					adapter = a;
				}
			}
		}

		/*Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
		COM_THROW_IF_FAILED( adapter->EnumOutputs( 0, &adapterOutput ) );

		UINT numModes;
		COM_THROW_IF_FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL ) );

		DXGI_MODE_DESC* displayModes = new DXGI_MODE_DESC[numModes];

		COM_THROW_IF_FAILED( adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModes ) );
		*/
		UINT numerator = 0, denominator = 1;
		/*for( UINT i = 0; i < numModes; i++ )
		{
			if( displayModes[i].Width == (UINT)wnd.GetWidth() && displayModes[i].Height == (UINT)wnd.GetHeight() )
			{
				numerator = displayModes[i].RefreshRate.Numerator;
				denominator = displayModes[i].RefreshRate.Denominator;
			}
		}

		delete[] displayModes;*/

		DXGI_ADAPTER_DESC adapterDesc;
		COM_THROW_IF_FAILED( adapter->GetDesc( &adapterDesc ) );

		m_szVideoCardDescription = adapterDesc.Description;
		m_iVideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024); // in mb

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

#ifdef _DEBUG
		UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
		UINT flags = 0;
#endif

		COM_THROW_IF_FAILED( D3D11CreateDeviceAndSwapChain(
			adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			flags,
			NULL, 0,
			D3D11_SDK_VERSION,
			&swapChainDesc, &m_pSwapChain,
			&m_pDevice, NULL, &m_pDeviceContext ) );

		BAT_LOG( "Using video card '{}' with {}MB VRAM",
			Bat::WideToString( m_szVideoCardDescription ),
			m_iVideoCardMemory );
		BAT_LOG( "Device feature level: {}", FeatureLevel2String( m_pDevice->GetFeatureLevel() ) );

		if( m_pDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0 )
		{
			BAT_ERROR( "Your feature level is unsupported, exiting..." );
			MessageBoxA( NULL, "Device feature level not high enough", "Error", MB_ICONERROR );
		}

		ID3D11Texture2D* pBackBuffer;
		COM_THROW_IF_FAILED( m_pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer ) );

		COM_THROW_IF_FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView ) );

		pBackBuffer->Release();
		pBackBuffer = nullptr;

		//Describe our Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = (UINT)wnd.GetWidth();
		depthStencilDesc.Height = (UINT)wnd.GetHeight();
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		COM_THROW_IF_FAILED( m_pDevice->CreateTexture2D( &depthStencilDesc, NULL, &m_pDepthStencilBuffer ) );
		COM_THROW_IF_FAILED( m_pDevice->CreateDepthStencilView( m_pDepthStencilBuffer.Get(), NULL, &m_pDepthStencilView ) );

		m_pDeviceContext->OMSetRenderTargets( 1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get() );

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

		COM_THROW_IF_FAILED( m_pDevice->CreateRasterizerState( &rasterDesc, &m_pRasterState ) );

		//Create depth stencil state enabled/disabled states
		D3D11_DEPTH_STENCIL_DESC depthstencildesc{};

		depthstencildesc.DepthEnable = true;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		COM_THROW_IF_FAILED( m_pDevice->CreateDepthStencilState( &depthstencildesc, &m_pDepthStencilEnabledState ) );

		depthstencildesc.DepthEnable = false;

		COM_THROW_IF_FAILED( m_pDevice->CreateDepthStencilState( &depthstencildesc, &m_pDepthStencilDisabledState ) );

		m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilEnabledState.Get(), 0 );
		m_pDeviceContext->RSSetState( m_pRasterState.Get() );

		BindViewport( wnd.GetWidth(), wnd.GetHeight() );
		m_iViewportWidth = wnd.GetWidth();
		m_iViewportHeight = wnd.GetHeight();

#ifdef _DEBUG
		// create the info queue
		typedef HRESULT (WINAPI * LPDXGIGETDEBUGINTERFACE)(REFIID, void ** );

		HMODULE dxgidebug = LoadLibraryEx( "dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 );
		if( dxgidebug )
		{
			auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
				reinterpret_cast<void*>( GetProcAddress( dxgidebug, "DXGIGetDebugInterface" ) ) );

			if( SUCCEEDED( dxgiGetDebugInterface( IID_PPV_ARGS( m_pInfoQueue.GetAddressOf() ) ) ) )
			{
				//m_pInfoQueue->SetMuteDebugOutput( DXGI_DEBUG_ALL, TRUE );
				m_pInfoQueue->ClearStoredMessages( DXGI_DEBUG_ALL );
				m_pInfoQueue->ClearRetrievalFilter( DXGI_DEBUG_ALL );
				m_pInfoQueue->ClearStorageFilter( DXGI_DEBUG_ALL );

				DXGI_INFO_QUEUE_FILTER filter {0};

				std::vector<DXGI_INFO_QUEUE_MESSAGE_SEVERITY> severity_filter_list = {
					// list of severities to filter out
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,
					// DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION
				};

				if( !severity_filter_list.empty() )
				{
					filter.DenyList.NumSeverities = (UINT)severity_filter_list.size();
					filter.DenyList.pSeverityList = severity_filter_list.data();
				}

				std::vector<DXGI_INFO_QUEUE_MESSAGE_ID> message_filter_list = {
					D3D11_MESSAGE_ID_DEVICE_DRAW_SHADERRESOURCEVIEW_NOT_SET
				};

				if( !message_filter_list.empty() )
				{
					filter.DenyList.NumIDs = (UINT)message_filter_list.size();
					filter.DenyList.pIDList = message_filter_list.data();
				}

				m_pInfoQueue->AddStorageFilterEntries( DXGI_DEBUG_ALL, &filter );
				m_pInfoQueue->AddRetrievalFilterEntries( DXGI_DEBUG_ALL, &filter );

				BAT_TRACE( "Initialized debug layer info queue" );
			}
		}
#endif
	}

	ID3D11Device* D3DClass::GetDevice() const
	{
		return m_pDevice.Get();
	}

	ID3D11DeviceContext* D3DClass::GetDeviceContext() const
	{
		return m_pDeviceContext.Get();
	}

	ID3D11RenderTargetView* D3DClass::GetRenderTargetView() const
	{
		return m_pRenderTargetView.Get();
	}

	ID3D11DepthStencilView* D3DClass::GetDepthStencilView() const
	{
		return m_pDepthStencilView.Get();
	}

	IDXGISwapChain* D3DClass::GetSwapChain() const
	{
		return m_pSwapChain.Get();
	}

	void D3DClass::GetVideoCardInfo( std::wstring& cardName, int& memory ) const
	{
		cardName = m_szVideoCardDescription;
		memory = m_iVideoCardMemory;
	}

	bool D3DClass::IsDepthStencilEnabled() const
	{
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pState = nullptr;
		UINT stencilref = 0;
		m_pDeviceContext->OMGetDepthStencilState( &pState, &stencilref );

		D3D11_DEPTH_STENCIL_DESC desc;
		pState->GetDesc( &desc );

		return desc.DepthEnable;
	}

	void D3DClass::SetDepthStencilEnabled( bool enable )
	{
		if( enable )
		{
			m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilEnabledState.Get(), 0 );
		}
		else
		{
			m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilDisabledState.Get(), 0 );
		}
	}

	void D3DClass::BindBackBuffer() const
	{
		m_pDeviceContext->OMSetRenderTargets( 1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get() );
	}

	void D3DClass::BindViewport( int width, int height )
	{
		// set up viewport
		g_Viewport.Width = (float)width;
		g_Viewport.Height = (float)height;
		g_Viewport.MinDepth = 0.0f;
		g_Viewport.MaxDepth = 1.0f;
		g_Viewport.TopLeftX = 0.0f;
		g_Viewport.TopLeftY = 0.0f;

		m_pDeviceContext->RSSetViewports( 1, &g_Viewport );
	}

	void D3DClass::ClearScene( float red, float green, float blue, float alpha )
	{
		const float colour[4] = { red, green, blue, alpha };
		m_pDeviceContext->ClearRenderTargetView( m_pRenderTargetView.Get(), colour );
		m_pDeviceContext->ClearDepthStencilView( m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
		BindViewport( m_iViewportWidth, m_iViewportHeight );
	}

	void D3DClass::PresentScene()
	{
		if( m_bVSyncEnabled )
		{
			m_pSwapChain->Present( 1, 0 );
		}
		else
		{
			m_pSwapChain->Present( 0, 0 );
		}

#ifdef _DEBUG
		// print debug layererrors from this frame
		if( m_pInfoQueue )
		{
			UINT64 nMessages = m_pInfoQueue->GetNumStoredMessages( DXGI_DEBUG_ALL );
			for (UINT64 i = 0 ; i < nMessages ; i++ )
			{
				SIZE_T length;
				m_pInfoQueue->GetMessageA( DXGI_DEBUG_ALL, i, NULL, &length);

				auto pMessage = (DXGI_INFO_QUEUE_MESSAGE*)malloc( length );
				m_pInfoQueue->GetMessageA( DXGI_DEBUG_ALL, i, pMessage, &length);

				switch( pMessage->Severity )
				{
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
						BAT_LOG( "{}", pMessage->pDescription );
						break;
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
						BAT_WARN( "{}", pMessage->pDescription );
						break;
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
					case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
						BAT_ERROR( "{}", pMessage->pDescription );
				}

				free( pMessage );
			}

			m_pInfoQueue->ClearStoredMessages( DXGI_DEBUG_ALL );
		}
#endif
	}

	void D3DClass::Resize( int width, int height )
	{
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

		m_iViewportWidth = width;
		m_iViewportHeight = height;

		m_pDeviceContext->OMSetRenderTargets( 0, NULL, NULL );

		m_pRenderTargetView.Reset();
		m_pDepthStencilView.Reset();
		m_pDepthStencilBuffer.Reset();

		HRESULT hr;
		COM_THROW_IF_FAILED( hr = m_pSwapChain->ResizeBuffers( 0, width, height, DXGI_FORMAT_UNKNOWN, 0 ) );

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBuffer;
		COM_THROW_IF_FAILED( m_pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&pBuffer ) );

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = (UINT)width;
		depthStencilDesc.Height = (UINT)height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		COM_THROW_IF_FAILED( m_pDevice->CreateTexture2D( &depthStencilDesc, NULL, &m_pDepthStencilBuffer ) );
		COM_THROW_IF_FAILED( m_pDevice->CreateDepthStencilView( m_pDepthStencilBuffer.Get(), NULL, &m_pDepthStencilView ) );

		COM_THROW_IF_FAILED( m_pDevice->CreateRenderTargetView( pBuffer.Get(), NULL, &m_pRenderTargetView ) );

		m_pDeviceContext->OMSetRenderTargets( 1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get() );

		D3D11_VIEWPORT viewport;
		viewport.Width = (float)width;
		viewport.Height = (float)height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		m_pDeviceContext->RSSetViewports( 1, &viewport );
	}

	static const char* FeatureLevel2String( D3D_FEATURE_LEVEL level )
	{
		switch( level )
		{
#define STRINGIFY_CASE( s ) case s: return STRINGIFY( s );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_9_1 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_9_2 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_9_3 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_10_0 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_10_1 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_11_0 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_11_1 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_12_0 );
			STRINGIFY_CASE( D3D_FEATURE_LEVEL_12_1 );
#undef STRINGIFY_CASE

			default:
				return "Unknown";
		}
	}
}