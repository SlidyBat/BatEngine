#pragma once

#include "IPipeline.h"
#include "Colour.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

namespace Bat
{
	struct CB_ColourPipelineMatrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	class ColourPipelineParameters : public IPipelineParameters
	{
	public:
		ColourPipelineParameters( const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}

		CB_ColourPipelineMatrix transform;
	};

	class ColourPipeline : public IPipeline
	{
	public:
		ColourPipeline( const std::string& vs_filename, const std::string& ps_filename );

		void BindParameters( IGPUContext* pContext, IPipelineParameters& pParameters ) override;
		void Render( IGPUContext* pContext, size_t vertexcount ) override;
		void RenderIndexed( IGPUContext* pContext, size_t indexcount ) override;
	private:
		ConstantBuffer<CB_ColourPipelineMatrix> m_cbufTransform;
	};
}