#include "PCH.h"
#include "ColourPipeline.h"

#include "Camera.h"
#include "ShaderManager.h"

namespace Bat
{
	void ColourPipeline::Render( IGPUContext* pContext, const Mesh& mesh, const Camera& camera, const Mat4& world_transform )
	{
		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/ColourVS.hlsl" );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/ColourPS.hlsl" );

		CB_ColourPipelineMatrix transform;
		transform.world = world_transform;
		transform.viewproj = camera.GetViewMatrix() * camera.GetProjectionMatrix();

		m_cbufTransform.Update( pContext, transform );

		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );

		mesh.Bind( pContext, pVertexShader );

		pContext->DrawIndexed( mesh.GetIndexCount() );
	}
}