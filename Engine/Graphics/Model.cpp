#include "Model.h"

#include "Graphics.h"
#include "TextureShader.h"

namespace Bat
{
	TexturedModel::TexturedModel( TexMesh mesh )
	{
		m_Meshes.emplace_back( mesh );
	}
	TexturedModel::TexturedModel( std::vector<TexMesh> meshes )
		:
		Model(),
		m_Meshes( std::move( meshes ) )
	{}

	void TexturedModel::Draw( IShader* pShader ) const
	{
		auto vp = DirectX::XMMatrixTranspose( g_pGfx->GetCamera()->GetViewMatrix() * g_pGfx->GetCamera()->GetProjectionMatrix() );
		auto w = DirectX::XMMatrixTranspose( GetWorldMatrix() );

		for( const auto& mesh : m_Meshes )
		{
			mesh.Bind();
			TextureShaderParameters params( w, vp, mesh.GetTexture() );
			pShader->BindParameters( &params );
			pShader->RenderIndexed( (UINT)mesh.GetIndexCount() );
		}
	}
}