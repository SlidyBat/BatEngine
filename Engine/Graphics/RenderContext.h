#pragma once

#include <d3d11.h>



namespace Bat
{
	class D3DClass;

	enum class ShaderType
	{
		VERTEX_SHADER,
		PIXEL_SHADER
	};

	class RenderContext
	{
		friend class Graphics;
	public:
		static ID3D11Device* GetDevice();
		static ID3D11DeviceContext* GetDeviceContext();
		static ID3D11RenderTargetView* GetBackBufferView();
		static ID3D11DepthStencilView* GetDepthStencilView();

		static void BindSamplers( const ShaderType shader );
		static void AddSampler( const D3D11_SAMPLER_DESC& sampler_desc );
	private:
		static void SetD3DClass( const D3DClass& d3d );
	};
}