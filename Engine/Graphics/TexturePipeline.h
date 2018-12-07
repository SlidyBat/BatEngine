#pragma once

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
	struct CB_TexturePipelineMatrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	class TexturedModel : public IModel
	{
	public:
		TexturedModel( const Mesh& mesh );
		TexturedModel( std::vector<Mesh> meshes );

		virtual void Draw( IPipeline* pPipeline ) const override;
	private:
		std::vector<Mesh> m_Meshes;
	};

	class ScreenQuadModel : public IModel
	{
	public:
		ScreenQuadModel( ID3D11ShaderResourceView* pTexture );

		virtual void Draw( IPipeline* pPipeline ) const override;
		Mesh GetMesh() const { return m_Mesh; }
	private:
		ID3D11ShaderResourceView* m_pTexture;
		Mesh m_Mesh;
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