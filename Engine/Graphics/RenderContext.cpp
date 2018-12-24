#include "PCH.h"
#include "RenderContext.h"

#include "Window.h"

#include "D3DClass.h"

namespace Bat
{
	const D3DClass* RenderContext::s_pD3DClass = nullptr;

	ID3D11Device* RenderContext::GetDevice()
	{
		return s_pD3DClass->GetDevice();
	}

	ID3D11DeviceContext* RenderContext::GetDeviceContext()
	{
		return s_pD3DClass->GetDeviceContext();
	}

	ID3D11RenderTargetView* RenderContext::GetBackBufferView()
	{
		return s_pD3DClass->GetRenderTargetView();
	}

	ID3D11DepthStencilView* RenderContext::GetDepthStencilView()
	{
		return s_pD3DClass->GetDepthStencilView();
	}

	void RenderContext::SetD3DClass( const D3DClass& d3d )
	{
		s_pD3DClass = &d3d;
	}
}