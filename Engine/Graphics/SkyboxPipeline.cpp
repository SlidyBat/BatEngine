#include "PCH.h"
#include "SkyboxPipeline.h"

#include "RenderContext.h"
#include "VertexTypes.h"
#include "COMException.h"

namespace Bat
{
	SkyboxPipeline::SkyboxPipeline( const std::string& vsFilename, const std::string& psFilename )
		:
		IPipeline( vsFilename, psFilename )
	{	
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_PixelShader.AddSampler( &samplerDesc );

		m_VertexShader.AddConstantBuffer<CB_SkyboxPipelineMatrix>();

		std::vector<Vec4> positions = {
			// Front Face
			Vec4( -1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f, -1.0f, -1.0f, 1.0f ),

			// Back Face            
			Vec4( -1.0f, -1.0f, 1.0f, 1.0f ),
			Vec4( 1.0f, -1.0f, 1.0f, 1.0f ),
			Vec4( 1.0f,  1.0f, 1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f, 1.0f, 1.0f ),

			// Top Face                
			Vec4( -1.0f, 1.0f, -1.0f, 1.0f ),
			Vec4( -1.0f, 1.0f,  1.0f, 1.0f ),
			Vec4( 1.0f, 1.0f,  1.0f, 1.0f ),
			Vec4( 1.0f, 1.0f, -1.0f, 1.0f ),

			// Bottom Face            
			Vec4( -1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f, -1.0f,  1.0f, 1.0f ),
			Vec4( -1.0f, -1.0f,  1.0f, 1.0f ),

			// Left Face            
			Vec4( -1.0f, -1.0f,  1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f,  1.0f, 1.0f ),
			Vec4( -1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4( -1.0f, -1.0f, -1.0f, 1.0f ),

			// Right Face            
			Vec4( 1.0f, -1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f,  1.0f, -1.0f, 1.0f ),
			Vec4( 1.0f,  1.0f,  1.0f, 1.0f ),
			Vec4( 1.0f, -1.0f,  1.0f, 1.0f ),
		};

		const std::vector<int> indices = {
			// Front Face
			2,  1,  0,
			3,  2,  0,

			// Back Face
			6,  5,  4,
			7,  6,  4,

			// Top Face
			10,  9, 8,
			11, 10, 8,

			// Bottom Face
			14, 13, 12,
			15, 14, 12,

			// Left Face
			18, 17, 16,
			19, 18, 16,

			// Right Face
			22, 21, 20,
			23, 22, 20
		};
			
		m_bufPositions.SetData( positions );
		m_bufIndices.SetData( indices );
	}

	void SkyboxPipeline::BindParameters( IPipelineParameters& pParameters )
	{
		auto pTextureParameters = static_cast<SkyboxPipelineParameters&>(pParameters);
		m_bufPositions.Bind( 0 );
		m_bufIndices.Bind();
		m_VertexShader.Bind();
		m_PixelShader.Bind();
		m_VertexShader.GetConstantBuffer( 0 ).SetData( &pTextureParameters.transform );
		m_PixelShader.SetResource( 0, pTextureParameters.texture );
	}

	void SkyboxPipeline::Render( UINT vertexcount )
	{
		ASSERT( false, "Skybox does not support non-indexed rendering" );
	}

	void SkyboxPipeline::RenderIndexed( UINT indexcount )
	{
		auto pDeviceContext = RenderContext::GetDeviceContext();
		pDeviceContext->DrawIndexed( (UINT)m_bufIndices.GetIndexCount(), 0, 0 );
	}
}