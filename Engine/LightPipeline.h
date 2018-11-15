#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

#include "IPipeline.h"
#include "Texture.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "IModel.h"
#include "Mesh.h"

namespace Bat
{
	struct CB_LightPipelineMatrix
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX viewproj;
	};

	class LightModel : public IModel
	{
	public:
		LightModel( Mesh mesh );
		LightModel( std::vector<Mesh> meshes );

		virtual void Draw( IPipeline* pPipeline ) const override;
	private:
		std::vector<Mesh> m_Meshes;
	};

	class LightPipelineParameters : public IPipelineParameters
	{
	public:
		LightPipelineParameters( const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& viewproj, Texture* pTexture )
			:
			pTexture( pTexture )
		{
			transform.world = world;
			transform.viewproj = viewproj;
		}
		CB_LightPipelineMatrix* GetTransformMatrix()
		{
			return &transform;
		}
		ID3D11ShaderResourceView* GetTextureView() const
		{
			return pTexture->GetTextureView();
		}
	private:
		CB_LightPipelineMatrix transform;
		Texture* pTexture;
	};

	class LightPipeline : public IPipeline
	{
	public:
		LightPipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters* pParameters ) override;
		void Render( UINT vertexcount ) override;
		void RenderIndexed( UINT indexcount ) override;
	private:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;
	};
}