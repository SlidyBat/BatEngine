#pragma once

#include <d3d11.h>



namespace Bat
{
	class D3DClass;

	class RenderContext
	{
		friend class Graphics;
	public:
		static ID3D11Device* GetDevice();
		static ID3D11DeviceContext* GetDeviceContext();
		static ID3D11RenderTargetView* GetBackBufferView();
		static ID3D11DepthStencilView* GetDepthStencilView();
	private:
		static void SetD3DClass( const D3DClass& d3d );
	private:
		static const D3DClass* s_pD3DClass;
	};
}