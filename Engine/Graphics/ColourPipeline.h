#pragma once

#include "IPipeline.h"
#include "Colour.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Mesh.h"

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
		CB_ColourPipelineMatrix* GetTransformMatrix()
		{
			return &transform;
		}
	private:
		CB_ColourPipelineMatrix transform;
	};

	class ColourPipeline : public IPipeline
	{
	public:
		ColourPipeline( const std::string& vsFilename, const std::string& psFilename );

		void BindParameters( IPipelineParameters& pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;
	};
}