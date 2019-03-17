#pragma once

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#include "PCH.h"

#include "Window.h"
#include <wrl.h>
#include <d3d11.h>

#ifdef _DEBUG
#include <initguid.h>
#include <d3d11sdklayers.h>
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib" )
#endif

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

		void ClearScene( float red, float green, float blue, float alpha );
		void PresentScene();
		void Resize( int width = 0, int height = 0 );

		ID3D11Device*             GetDevice() const;
		ID3D11DeviceContext*      GetDeviceContext() const;
		ID3D11RenderTargetView*   GetRenderTargetView() const;
		ID3D11DepthStencilView*   GetDepthStencilView() const;
		ID3D11ShaderResourceView* GetDepthShaderResourceView() const;
		IDXGISwapChain*           GetSwapChain() const;

		void GetVideoCardInfo( std::wstring& cardName, int& memory ) const;

		bool IsDepthStencilEnabled() const;
		void SetDepthStencilEnabled( bool enable );

		void BindBackBuffer() const;

		void FlushMessages();
	private:
		void BindViewport( int width, int height );
	private:
		bool						m_bVSyncEnabled;

		int							m_iVideoCardMemory;
		std::wstring				m_szVideoCardDescription;

		Microsoft::WRL::ComPtr<IDXGISwapChain>				m_pSwapChain;

		Microsoft::WRL::ComPtr<ID3D11Device>				m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>			m_pDeviceContext;

#ifdef _DEBUG
		Microsoft::WRL::ComPtr<IDXGIInfoQueue>				m_pInfoQueue;
#endif

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_pRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_pRasterState;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_pDepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pDepthShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		m_pDepthStencilEnabledState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		m_pDepthStencilDisabledState;

		int m_iViewportWidth;
		int m_iViewportHeight;
	};
}