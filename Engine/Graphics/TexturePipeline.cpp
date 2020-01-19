#include "PCH.h"
#include "TexturePipeline.h"

#include "VertexTypes.h"
#include "COMException.h"
#include "Material.h"
#include "ShaderManager.h"
#include "Camera.h"

namespace Bat
{
	void TexturePipeline::Render( IGPUContext* pContext, const Mesh& mesh, const Camera& camera, ITexture* pTexture, const DirectX::XMMATRIX& world_transform )
	{
		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/TextureVS.hlsl" );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/TexturePS.hlsl" );

		CB_TexturePipelineMatrix transform;
		transform.world = world_transform;
		transform.viewproj = camera.GetViewMatrix() * camera.GetProjectionMatrix();

		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );
		pContext->BindTexture( pTexture, PS_TEX_SLOT_0 );

		mesh.Bind( pContext, pVertexShader );

		pContext->DrawIndexed( mesh.GetIndexCount() );
	}
}