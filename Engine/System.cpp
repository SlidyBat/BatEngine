#include "System.h"
#include "GDIPManager.h"
#include "Resource.h"

System::System()
{
	int screenWidth = 0;
	int screenHeight = 0;
	InitializeWindows( screenWidth, screenHeight );

	assert( screenWidth > 0 );
	assert( screenHeight > 0 );

	game = std::make_unique<Game>( screenWidth, screenHeight, m_hWnd );
}

System::~System()
{
	ShutdownWindows();
}

void System::Run()
{
	GDIPManager gdipm;

	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	bool done = false;
	while( !done )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if( msg.message == WM_QUIT )
		{
			done = true;
		}
		else
		{
			if( !Frame() )
			{
				done = true;
			}
		}
	}
}

LRESULT System::MessageHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			Input::KeyDown( (size_t)wParam );
			return 0;
		}
	case WM_KEYUP:
		{
			Input::KeyUp( (size_t)wParam );
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			Input::MouseButtonDown( Input::MouseButton::Left );
			return 0;
		}
	case WM_LBUTTONUP:
		{
			Input::MouseButtonUp( Input::MouseButton::Left );
			return 0;
		}
	case WM_LBUTTONDBLCLK:
		{
			Input::MouseButtonDblClick( Input::MouseButton::Left );
			return 0;
		}
	case WM_RBUTTONDOWN:
		{
			Input::MouseButtonDown( Input::MouseButton::Right );
			return 0;
		}
	case WM_RBUTTONUP:
		{
			Input::MouseButtonUp( Input::MouseButton::Right );
			return 0;
		}
	case WM_RBUTTONDBLCLK:
		{
			Input::MouseButtonDblClick( Input::MouseButton::Right );
			return 0;
		}
	case WM_MBUTTONDOWN:
		{
			Input::MouseButtonDown( Input::MouseButton::Middle );
			return 0;
		}
	case WM_MBUTTONUP:
		{
			Input::MouseButtonUp( Input::MouseButton::Middle );
			return 0;
		}
	case WM_MBUTTONDBLCLK:
		{
			Input::MouseButtonDblClick( Input::MouseButton::Middle );
			return 0;
		}
	case WM_XBUTTONDOWN:
		{
			if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 )
			{
				Input::MouseButtonDown( Input::MouseButton::X1 );
			}
			else if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON2 )
			{
				Input::MouseButtonDown( Input::MouseButton::X2 );
			}
			return 0;
		}
	case WM_XBUTTONUP:
		{
			if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 )
			{
				Input::MouseButtonUp( Input::MouseButton::X1 );
			}
			else if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON2 )
			{
				Input::MouseButtonUp( Input::MouseButton::X2 );
			}
			return 0;
		}
	case WM_XBUTTONDBLCLK:
		{
			if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 )
			{
				Input::MouseButtonDblClick( Input::MouseButton::X1 );
			}
			else if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON2 )
			{
				Input::MouseButtonDblClick( Input::MouseButton::X2 );
			}
			return 0;
		}
	default:
		{
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
	}
}

bool System::Frame()
{
	if( Input::IsKeyPressed( VK_ESCAPE ) )
	{
		return false;
	}

	game->Run();

	return true;
}

void System::InitializeWindows( int& screenWidth, int& screenHeight )
{
	m_hInstance = GetModuleHandle( NULL );
	m_szApplicationName = "SlidyEngine";

	WNDCLASSEX wc;

	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 32, 32, 0 );
	wc.hIconSm = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 16, 16, 0 );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_szApplicationName;
	wc.cbSize = sizeof( WNDCLASSEX );

	RegisterClassEx( &wc );

	int posX, posY;
	if( Graphics::FullScreen )
	{
		screenWidth = GetSystemMetrics( SM_CXSCREEN );
		screenHeight = GetSystemMetrics( SM_CYSCREEN );

		DEVMODE dmScreenSettings;
		memset( &dmScreenSettings, 0, sizeof( dmScreenSettings ) );
		dmScreenSettings.dmSize = sizeof( dmScreenSettings );
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;

		ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN );

		posX = 0;
		posY = 0;

		m_hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			m_szApplicationName,
			m_szApplicationName,
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP,
			posX,
			posY,
			screenWidth,
			screenHeight,
			NULL,
			NULL,
			m_hInstance,
			this );
	}
	else
	{
		screenWidth = Graphics::ScreenWidth;
		screenHeight = Graphics::ScreenHeight;

		// centre
		posX = ( GetSystemMetrics( SM_CXSCREEN ) - screenWidth ) / 2;
		posY = ( GetSystemMetrics( SM_CYSCREEN ) - screenHeight ) / 2;

		RECT windowRect = { 0, 0, screenWidth, screenHeight };
		AdjustWindowRect( &windowRect, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW, false );

		m_hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			m_szApplicationName,
			m_szApplicationName,
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW,
			posX,
			posY,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			m_hInstance,
			this );
	}

	ShowWindow( m_hWnd, SW_SHOW );
	SetForegroundWindow( m_hWnd );
	SetFocus( m_hWnd );
}

void System::ShutdownWindows()
{
	ShowCursor( true );

	if( Graphics::FullScreen )
	{
		ChangeDisplaySettings( NULL, 0 );
	}

	DestroyWindow( m_hWnd );
	m_hWnd = NULL;

	UnregisterClass( m_szApplicationName, m_hInstance );
	m_hInstance = NULL;
}

LRESULT CALLBACK System::HandleMsgSetup( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_NCCREATE:
		{
			const CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>( lParam );

			System* pSystem = reinterpret_cast<System*>( pCreate->lpCreateParams );
			assert( pSystem );

			SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pSystem ) );
			SetWindowLongPtr( hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( &HandleMsgThunk ) );

			return pSystem->MessageHandler( hWnd, uMsg, wParam, lParam );
		}
	default:
		{
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
	}
}

LRESULT CALLBACK System::HandleMsgThunk( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	System* pSystem = reinterpret_cast<System*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
	return pSystem->MessageHandler( hWnd, uMsg, wParam, lParam );
}
