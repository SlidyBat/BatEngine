#pragma once

#include "SlidyWin.h"
#include "Game.h"
#include "Input.h"

class System
{
public:
	System() = default;
	System( const System& src ) = delete;
	System& operator=( const System& src ) = delete;
	System( System&& donor ) = delete;
	System& operator=( System&& donor ) = delete;
	~System();

	bool Initialize();
	void Run();

	LRESULT CALLBACK MessageHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
private:
	bool Frame();
	void InitializeWindows( int& screenWidth, int& screenHeight );
	void ShutdownWindows();

	static LRESULT CALLBACK HandleMsgSetup( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK HandleMsgThunk( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
private:
	LPCSTR		m_szApplicationName;
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;

	Game game;
};