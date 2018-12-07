#pragma once

#include "PCH.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include "IPipeline.h"
#include "Texture.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "IModel.h"
#include "Mesh.h"

namespace Bat
{
	struct CB_SkyboxPipelineMatrix
	{
		DirectX::XMMATRIX viewproj;
	};

	class SkyboxPipelineParameters : public IPipelineParameters
	{
	public:
		SkyboxPipelineParameters( const DirectX::XMMATRIX& viewproj, ID3D11ShaderResourceView* pTexture )
			:
			texture( pTexture ),
			transform( viewproj )
		{}
	public:
		DirectX::XMMATRIX transform;
		ID3D11ShaderResourceView* texture;
	};

	class SkyboxPipeline : public IPipeline
	{
	public:
		SkyboxPipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters* pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;
	private:
		VertexBuffer<Vec4> m_bufPositions;
		IndexBuffer m_bufIndices;
	};
}