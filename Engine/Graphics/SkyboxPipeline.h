#pragma once

#include "PCH.h"

#include "IGPUDevice.h"
#include "IPipeline.h"
#include "Texture.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	struct CB_SkyboxPipelineMatrix
	{
		DirectX::XMMATRIX viewproj;
	};

	class SkyboxPipelineParameters : public IPipelineParameters
	{
	public:
		SkyboxPipelineParameters( const DirectX::XMMATRIX& viewproj, ITexture* pTexture )
			:
			texture( pTexture ),
			transform( viewproj )
		{}
	public:
		DirectX::XMMATRIX transform;
		ITexture* texture;
	};

	class SkyboxPipeline : public IPipeline
	{
	public:
		SkyboxPipeline( const std::string& vsFilename, const std::string& psFilename );

		void BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters ) override;
		void Render( IGPUContext* pContext, size_t vertexcount ) override;
		void RenderIndexed( IGPUContext* pContext, size_t indexcount ) override;
	private:
		VertexBuffer<Vec4> m_bufPositions;
		IndexBuffer m_bufIndices;
		ConstantBuffer<CB_SkyboxPipelineMatrix> m_cbufTransform;
	};
}