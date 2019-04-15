#pragma once

#include "IPipeline.h"
#include "Texture.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	struct CB_TexturePipelineMatrix
	{
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX viewproj = DirectX::XMMatrixIdentity();
	};

	class TexturePipelineParameters : public IPipelineParameters
	{
	public:
		TexturePipelineParameters( const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj, ITexture* pTexture )
			:
			texture( pTexture )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}
	public:
		CB_TexturePipelineMatrix transform;
		ITexture* texture;
	};

	class TexturePipeline : public IPipeline
	{
	public:
		TexturePipeline( const std::string& vsFilename, const std::string& psFilename );

		void BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters ) override;
		void Render( IGPUContext* pContext, size_t vertexcount ) override;
		void RenderIndexed( IGPUContext* pContext, size_t indexcount ) override;
	private:
		ConstantBuffer<CB_TexturePipelineMatrix> m_cbufTransform;
	};
}