#include "PCH.h"
#include "SkyboxPipeline.h"

#include "VertexTypes.h"
#include "COMException.h"
#include "ShaderManager.h"
#include "Camera.h"

namespace Bat
{
	SkyboxPipeline::SkyboxPipeline()
		:
		m_bufPositions( {
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
		Vec4( 1.0f, -1.0f,  1.0f, 1.0f ) } ),
		m_bufIndices( {
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
		23, 22, 20 } )
	{}

	void SkyboxPipeline::Render( IGPUContext* pContext, const Camera& camera, ITexture* pTexture )
	{
		IVertexShader* pVertexShader = ResourceManager::GetVertexShader( "Graphics/Shaders/SkyboxVS.hlsl" );
		IPixelShader* pPixelShader = ResourceManager::GetPixelShader( "Graphics/Shaders/SkyboxPS.hlsl" );

		auto pos = camera.GetPosition();
		auto w = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );

		CB_SkyboxPipelineMatrix transform;
		transform.viewproj = w * camera.GetViewMatrix() * camera.GetProjectionMatrix();
		m_cbufTransform.Update( pContext, transform );

		pContext->SetVertexBuffer( m_bufPositions, 0 );
		pContext->SetIndexBuffer( m_bufIndices );
		pContext->SetVertexShader( pVertexShader );
		pContext->SetPixelShader( pPixelShader );
		pContext->SetConstantBuffer( ShaderType::VERTEX, m_cbufTransform, VS_CBUF_TRANSFORMS );
		pContext->BindTexture( pTexture, 0 );

		pContext->SetPrimitiveTopology( PrimitiveTopology::TRIANGLELIST );
		pContext->DrawIndexed( m_bufIndices->GetIndexCount() );
	}
}