#pragma once

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

class D3DClass
{
public:
	D3DClass() = default;
	~D3DClass();
	D3DClass( const D3DClass& src ) = delete;
	D3DClass& operator=( const D3DClass& src ) = delete;
	D3DClass( D3DClass&& donor ) = delete;
	D3DClass& operator=( D3DClass&& donor ) = delete;

	bool Initialize( int screenWidth, int screenHeight, bool fullscreen, HWND hWnd, bool vsyncEnabled, float screendepth, float screennear );

	void BeginScene( float red, float green, float blue, float alpha );
	void EndScene();

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;

	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMMATRIX GetWorldMatrix() const;
	DirectX::XMMATRIX GetOrthoMatrix() const;

	void GetVideoCardInfo( std::wstring& cardName, int& memory ) const;
private:
	bool						m_bVSyncEnabled;

	int							m_iVideoCardMemory;
	std::wstring				m_szVideoCardDescription;

	IDXGISwapChain*				m_pSwapChain = nullptr;
	ID3D11Device*				m_pDevice = nullptr;
	ID3D11DeviceContext*		m_pDeviceContext = nullptr;
	ID3D11RenderTargetView*		m_pRenderTargetView = nullptr;
	ID3D11Texture2D*			m_pDepthStencilBuffer = nullptr;
	ID3D11DepthStencilState*	m_pDepthStencilState = nullptr;
	ID3D11DepthStencilView*		m_pDepthStencilView = nullptr;
	ID3D11RasterizerState*		m_pRasterState = nullptr;

	DirectX::XMMATRIX			m_matProjectionMatrix;
	DirectX::XMMATRIX			m_matWorldMatrix;
	DirectX::XMMATRIX			m_matOrthoMatrix;
};