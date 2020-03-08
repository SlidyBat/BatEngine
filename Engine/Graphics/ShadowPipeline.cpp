#include "PCH.h"
#include "ShadowPipeline.h"

#include "ShaderManager.h"
#include "Mesh.h"

namespace Bat
{
	void ShadowPipeline::Render( IGPUContext* pContext, const Mesh& mesh, const Mat4& viewproj, const Mat4& world_transform )
	{
		auto macros = ShaderManager::BuildMacrosForMesh( mesh );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/ShadowMapVS.hlsl", macros );
		pContext->SetVertexShader( pVertexShader );
		
		// If there is some alpha masking going on then we need a pixel shader that discards low alpha pixels
		const Material& material = mesh.GetMaterial();
		if( material.GetAlphaMode() == AlphaMode::MASK )
		{
			IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/ShadowMapPS.hlsl", macros );
			pContext->SetPixelShader( pPixelShader );
			BindMaterial( pContext, material );
		}
		else
		{
			pContext->SetPixelShader( nullptr );
		}

		BindTransforms( pContext, viewproj, world_transform );

		mesh.Bind( pContext, pVertexShader );

		pContext->DrawIndexed( mesh.GetIndexCount() );
	}

	void ShadowPipeline::BindTransforms( IGPUContext* pContext, const Mat4& viewproj, const Mat4& world_transform )
	{
		CB_ShadowPipelineMatrix transform;
		transform.world = world_transform;
		transform.viewproj = viewproj;
		m_cbufTransform.Update( pContext, transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );
	}
	void ShadowPipeline::BindMaterial( IGPUContext* pContext, const Material& material )
	{
		CB_ShadowPipelineMaterial cbuf;
		cbuf.BaseColourFactor = material.GetBaseColourFactor();
		cbuf.HasBaseColourTexture = ( material.GetBaseColour() != nullptr );
		cbuf.AlphaCutoff = material.GetAlphaCutoff();
		m_cbufMaterial.Update( pContext, cbuf );
		pContext->SetConstantBuffer( ShaderType::PIXEL, m_cbufMaterial, PS_CBUF_SLOT_0 );

		pContext->BindTexture( material.GetBaseColour()->Get(), PS_TEX_SLOT_0 );
	}
}
