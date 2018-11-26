#pragma once

#include "IPipeline.h"
#include "Colour.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "IModel.h"
#include "Mesh.h"

namespace Bat
{
	struct CB_ColourPipelineMatrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	class ColouredModel : public IModel
	{
	public:
		ColouredModel( const Mesh& mesh );
		ColouredModel( std::vector<Mesh> meshes );

		virtual void Draw( IPipeline* pPipeline ) const override;
	private:
		std::vector<Mesh> m_Meshes;
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
		ColourPipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters* pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;
	private:
	};
}