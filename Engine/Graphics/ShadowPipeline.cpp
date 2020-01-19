#include "PCH.h"
#include "ShadowPipeline.h"

#include "ShaderManager.h"
#include "Mesh.h"

namespace Bat
{
	void ShadowPipeline::Render( IGPUContext* pContext, const Mesh& mesh, const DirectX::XMMATRIX& viewproj, const DirectX::XMMATRIX& world_transform )
	{
		auto macros = ShaderManager::BuildMacrosForMesh( mesh );

		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/ShadowMapVS.hlsl", macros );
		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( nullptr );

		BindTransforms( pContext, viewproj, world_transform );

		mesh.Bind( pContext, pVertexShader );

		pContext->DrawIndexed( mesh.GetIndexCount() );
	}

	void ShadowPipeline::BindTransforms( IGPUContext* pContext, const DirectX::XMMATRIX& viewproj, const DirectX::XMMATRIX& world_transform )
	{
		CB_ShadowPipelineMatrix transform;
		transform.world = world_transform;
		transform.viewproj = viewproj;
		m_cbufTransform.Update( pContext, transform );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );
	}
}
