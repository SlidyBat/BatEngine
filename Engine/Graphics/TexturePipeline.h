#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include "IPipeline.h"
#include "Texture.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Mesh.h"

namespace Bat
{
	struct CB_TexturePipelineMatrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	class TexturePipelineParameters : public IPipelineParameters
	{
	public:
		TexturePipelineParameters( const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj, ID3D11ShaderResourceView* pTexture )
			:
			texture( pTexture )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}
	public:
		CB_TexturePipelineMatrix transform;
		ID3D11ShaderResourceView* texture;
	};

	class TexturePipeline : public IPipeline
	{
	public:
		TexturePipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters* pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;
	};
}