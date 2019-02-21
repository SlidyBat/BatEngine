#pragma once

#include "PCH.h"
#include "Input.h"

namespace Bat
{
	class Window : public EventDispatcher
	{
	public:
		Window( const Vei2& pos, int width, int height, const std::string& name, bool fullscreen = false );
		Window( int width, int height, const std::string& name, bool fullscreen = false )
			:
			Window( { 50, 50 }, width, height, name, fullscreen )
		{}
		~Window() noexcept;

		void Kill();
		bool IsActive() const;
		bool IsMinimized() const;
		void ShowMessageBox( const std::string& title, const std::string& msg, UINT type ) const;
		bool ProcessMessage();

		bool IsFullscreen() const
		{
			return m_bFullscreen;
		}
		int GetWidth() const
		{
			return m_iWidth;
		}
		int GetHeight() const
		{
			return m_iHeight;
		}
		Vei2 GetPosition() const
		{
			return m_Pos;
		}
		int GetStyle() const
		{
			return m_dwStyle;
		}
		HWND GetHandle() const
		{
			return m_hWnd;
		}
	public:
		Input input;
	private:
		LRESULT CALLBACK HandleMsg( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		static LRESULT CALLBACK HandleMsgSetup( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		static LRESULT CALLBACK HandleMsgThunk( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	private:
		int m_iWidth;
		int m_iHeight;
		Vei2 m_Pos;
		DWORD m_dwStyle;

		bool m_bFullscreen;

		bool m_bDestroyed = false;

		std::string	m_szApplicationName;
		HINSTANCE	m_hInstance;
		HWND		m_hWnd;
	};
}