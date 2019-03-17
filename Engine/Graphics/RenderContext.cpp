#include "PCH.h"
#include "RenderContext.h"

#include "Window.h"
#include "COMException.h"

#include "D3DClass.h"

namespace Bat
{
	static D3DClass* g_pD3DClass = nullptr;
	static std::vector<ID3D11SamplerState*> g_pSamplerStates;

	ID3D11Device* RenderContext::GetDevice()
	{
		return g_pD3DClass->GetDevice();
	}

	ID3D11DeviceContext* RenderContext::GetDeviceContext()
	{
		return g_pD3DClass->GetDeviceContext();
	}

	ID3D11RenderTargetView* RenderContext::GetBackBufferView()
	{
		return g_pD3DClass->GetRenderTargetView();
	}

	ID3D11DepthStencilView* RenderContext::GetDepthStencilView()
	{
		return g_pD3DClass->GetDepthStencilView();
	}

	ID3D11ShaderResourceView* RenderContext::GetDepthShaderResourceView()
	{
		return g_pD3DClass->GetDepthShaderResourceView();
	}

	IDXGISwapChain* RenderContext::GetSwapChain()
	{
		return g_pD3DClass->GetSwapChain();
	}

	void RenderContext::SetD3DClass( D3DClass& d3d )
	{
		g_pD3DClass = &d3d;
	}

	void RenderContext::BindSamplers( const ShaderType shader )
	{
		if( !g_pSamplerStates.empty() )
		{
			if( shader == ShaderType::VERTEX_SHADER )
			{
				GetDeviceContext()->VSSetSamplers( 0, (UINT)g_pSamplerStates.size(), g_pSamplerStates.data() );
			}
			else if( shader == ShaderType::PIXEL_SHADER )
			{
				GetDeviceContext()->PSSetSamplers( 0, (UINT)g_pSamplerStates.size(), g_pSamplerStates.data() );
			}
		}
	}

	void RenderContext::AddSampler( const D3D11_SAMPLER_DESC& sampler_desc )
	{
		auto pDevice = RenderContext::GetDevice();

		ID3D11SamplerState* pSamplerState;
		COM_THROW_IF_FAILED( pDevice->CreateSamplerState( &sampler_desc, &pSamplerState ) );
		g_pSamplerStates.emplace_back( pSamplerState );

		ASSERT( g_pSamplerStates.size() <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, "Too many sampler states!" );
	}

	bool RenderContext::IsDepthStencilEnabled()
	{
		return g_pD3DClass->IsDepthStencilEnabled();
	}

	void RenderContext::SetDepthStencilEnabled( bool enabled )
	{
		g_pD3DClass->SetDepthStencilEnabled( enabled );
	}

	void RenderContext::FlushMessages()
	{
		g_pD3DClass->FlushMessages();
	}
}