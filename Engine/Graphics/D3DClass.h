#pragma once

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#include "Window.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

namespace Bat
{
	class D3DClass
	{
	public:
		D3DClass( Window& wnd, bool vsyncEnabled, float screendepth, float screennear );
		D3DClass( const D3DClass& src ) = delete;
		D3DClass& operator=( const D3DClass& src ) = delete;
		D3DClass( D3DClass&& donor ) = delete;
		D3DClass& operator=( D3DClass&& donor ) = delete;

		void BeginScene( float red, float green, float blue, float alpha );
		void EndScene();
		void Resize( int width = 0, int height = 0 );

		ID3D11Device* GetDevice() const;
		ID3D11DeviceContext* GetDeviceContext() const;

		void GetVideoCardInfo( std::wstring& cardName, int& memory ) const;
	private:
		bool						m_bVSyncEnabled;

		int							m_iVideoCardMemory;
		std::wstring				m_szVideoCardDescription;

		Microsoft::WRL::ComPtr<IDXGISwapChain>				m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D11Device>				m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>			m_pDeviceContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_pRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_pRasterState;
	};
}