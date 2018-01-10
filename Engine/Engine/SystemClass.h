#pragma once

#include "SlidyWin.h"
#include "Graphics.h"
#include "Input.h"

class SystemClass
{
public:
	SystemClass() = default;
	SystemClass( const SystemClass& src ) = delete;
	SystemClass& operator=( const SystemClass& src ) = delete;
	SystemClass( SystemClass&& donor ) = delete;
	SystemClass& operator=( SystemClass&& donor ) = delete;
	~SystemClass();

	bool Initialize();
	void Run();

	LRESULT CALLBACK MessageHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
private:
	bool Frame();
	void InitializeWindows( int& screenWidth, int& screenHeight );
	void ShutdownWindows();
private:
	LPCSTR		m_szApplicationName;
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;

	Input		input;
	Graphics	gfx;
};

static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static SystemClass* ApplicationHandle = nullptr;