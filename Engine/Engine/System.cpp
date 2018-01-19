#include "System.h"
#include "GDIPManager.h"

System::~System()
{
	ShutdownWindows();
}

bool System::Initialize()
{
	int screenWidth = 0;
	int screenHeight = 0;
	InitializeWindows( screenWidth, screenHeight );

	if( !game.Initialize( screenWidth, screenHeight, m_hWnd ) )
	{
		return false;
	}

	return true;
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

	game.Run();

	return true;
}

void System::InitializeWindows( int& screenWidth, int& screenHeight )
{
	ApplicationHandle = this;

	m_hInstance = GetModuleHandle( NULL );
	m_szApplicationName = "SlidyEngine";

	WNDCLASSEX wc;

	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon( m_hInstance, "slidy.ico" );
	wc.hIconSm = wc.hIcon;
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
			NULL );
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
			NULL );
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

	ApplicationHandle = nullptr;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_DESTROY:
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		return 0;
	}
	default:
	{
		return ApplicationHandle->MessageHandler( hWnd, uMsg, wParam, lParam );
	}
	}
}
