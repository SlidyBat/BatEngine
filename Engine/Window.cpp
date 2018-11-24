#include "Window.h"
#include "Resource.h"
#include "BatAssert.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

namespace Bat
{
	Window::Window( const Vei2& pos, int width, int height, const std::string& name, bool fullscreen )
		:
		m_bFullscreen( fullscreen )
	{
		m_hInstance = GetModuleHandle( NULL );
		m_szApplicationName = name;

		WNDCLASSEX wc;

		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInstance;
		wc.hIcon = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 32, 32, 0 );
		wc.hIconSm = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 16, 16, 0 );
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
		wc.lpszMenuName = NULL;
		wc.lpszClassName = m_szApplicationName.c_str();
		wc.cbSize = sizeof( WNDCLASSEX );

		RegisterClassEx( &wc );

		if( fullscreen )
		{
			m_iWidth = GetSystemMetrics( SM_CXSCREEN );
			m_iHeight = GetSystemMetrics( SM_CYSCREEN );

			DEVMODE dmScreenSettings;
			memset( &dmScreenSettings, 0, sizeof( dmScreenSettings ) );
			dmScreenSettings.dmSize = sizeof( dmScreenSettings );
			dmScreenSettings.dmPelsWidth = (unsigned long)m_iWidth;
			dmScreenSettings.dmPelsHeight = (unsigned long)m_iHeight;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

			ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN );

			m_Pos = { 0, 0 };
			m_dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP;

			m_hWnd = CreateWindowEx(
				WS_EX_APPWINDOW,
				m_szApplicationName.c_str(),
				m_szApplicationName.c_str(),
				m_dwStyle,
				m_Pos.x,
				m_Pos.y,
				m_iWidth,
				m_iHeight,
				NULL,
				NULL,
				m_hInstance,
				this );
		}
		else
		{
			m_iWidth = width;
			m_iHeight = height;

			m_Pos = pos;
			m_dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;

			RECT windowRect = { 0, 0, m_iWidth, m_iHeight };
			AdjustWindowRect( &windowRect, m_dwStyle, false );

			m_hWnd = CreateWindowEx(
				WS_EX_APPWINDOW,
				m_szApplicationName.c_str(),
				m_szApplicationName.c_str(),
				m_dwStyle,
				m_Pos.x,
				m_Pos.y,
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

	Window::~Window() noexcept
	{
		ShowCursor( true );

		if( m_bFullscreen )
		{
			ChangeDisplaySettings( NULL, 0 );
		}

		DestroyWindow( m_hWnd );
		m_hWnd = NULL;

		UnregisterClass( m_szApplicationName.c_str(), m_hInstance );
		m_hInstance = NULL;
	}

	void Window::Kill()
	{
		m_bDestroyed = true;
	}

	bool Window::IsActive() const
	{
		return GetActiveWindow() == m_hWnd;
	}

	bool Window::IsMinimized() const
	{
		return IsIconic( m_hWnd ) != 0;
	}

	void Window::ShowMessageBox( const std::string& title, const std::string& msg, UINT type ) const
	{
		MessageBox( m_hWnd, msg.c_str(), title.c_str(), type );
	}

	bool Window::ProcessMessage()
	{
		if( m_bDestroyed )
		{
			return false;
		}

		MSG msg;

		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );

			if( msg.message == WM_QUIT )
			{
				return false;
			}
		}

		return true;
	}

	void Window::AddResizeListener( const std::function<void( int, int )>& callback )
	{
		m_ResizeListeners.push_back( callback );
	}

	LRESULT Window::HandleMsg( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		if( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) )
		{
			return true;
		}

		switch( uMsg )
		{
		case WM_DESTROY:
		{
			Kill();
			break;
		}
		case WM_KEYDOWN:
		{
			input.KeyDown( (size_t)wParam );
			break;
		}
		case WM_KEYUP:
		{
			input.KeyUp( (size_t)wParam );
			break;
		}
		case WM_LBUTTONDOWN:
		{
			input.MouseButtonDown( Input::MouseButton::Left );
			break;
		}
		case WM_LBUTTONUP:
		{
			input.MouseButtonUp( Input::MouseButton::Left );
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			input.MouseButtonDblClick( Input::MouseButton::Left );
			break;
		}
		case WM_RBUTTONDOWN:
		{
			input.MouseButtonDown( Input::MouseButton::Right );
			break;
		}
		case WM_RBUTTONUP:
		{
			input.MouseButtonUp( Input::MouseButton::Right );
			break;
		}
		case WM_RBUTTONDBLCLK:
		{
			input.MouseButtonDblClick( Input::MouseButton::Right );
			break;
		}
		case WM_MBUTTONDOWN:
		{
			input.MouseButtonDown( Input::MouseButton::Middle );
			break;
		}
		case WM_MBUTTONUP:
		{
			input.MouseButtonUp( Input::MouseButton::Middle );
			break;
		}
		case WM_MBUTTONDBLCLK:
		{
			input.MouseButtonDblClick( Input::MouseButton::Middle );
			break;
		}
		case WM_XBUTTONDOWN:
		{
			if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 )
			{
				input.MouseButtonDown( Input::MouseButton::X1 );
			}
			else if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON2 )
			{
				input.MouseButtonDown( Input::MouseButton::X2 );
			}
			break;
		}
		case WM_XBUTTONUP:
		{
			if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 )
			{
				input.MouseButtonUp( Input::MouseButton::X1 );
			}
			else if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON2 )
			{
				input.MouseButtonUp( Input::MouseButton::X2 );
			}
			break;
		}
		case WM_XBUTTONDBLCLK:
		{
			if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 )
			{
				input.MouseButtonDblClick( Input::MouseButton::X1 );
			}
			else if( GET_XBUTTON_WPARAM( wParam ) == XBUTTON2 )
			{
				input.MouseButtonDblClick( Input::MouseButton::X2 );
			}
			break;
		}
		case WM_SIZE: // called when window is resized
		{
			m_iWidth = LOWORD( lParam );
			m_iHeight = HIWORD( lParam );

			for( auto& cb : m_ResizeListeners )
			{
				cb( m_iWidth, m_iHeight );
			}

			break;
		}
		case WM_MOVE: // called when window is moved
		{
			m_Pos.x = LOWORD( lParam );
			m_Pos.y = HIWORD( lParam );
			break;
		}
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	LRESULT CALLBACK Window::HandleMsgSetup( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		switch( uMsg )
		{
		case WM_NCCREATE:
		{
			const CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);

			Window* pWnd = reinterpret_cast<Window*>(pCreate->lpCreateParams);
			ASSERT( pWnd, "Failed to get Window ptr" );

			SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd) );
			SetWindowLongPtr( hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HandleMsgThunk) );

			return pWnd->HandleMsg( hWnd, uMsg, wParam, lParam );
		}
		default:
		{
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
		}
	}

	LRESULT CALLBACK Window::HandleMsgThunk( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Window* pWnd = reinterpret_cast<Window*>(GetWindowLongPtr( hWnd, GWLP_USERDATA ));
		return pWnd->HandleMsg( hWnd, uMsg, wParam, lParam );
	}
}