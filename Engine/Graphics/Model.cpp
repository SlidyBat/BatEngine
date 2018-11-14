#include "Model.h"

#include "TextureShader.h"

namespace Bat
{
	TexturedModel::TexturedModel( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IShader* pShader, Camera* pCamera, Texture* pTexture, const std::vector<TexVertex>& verts, const std::vector<int>& indices )
		:
		m_VertexBuffer( pDevice, verts ),
		m_IndexBuffer( pDevice, indices ),
		m_pCamera( pCamera ),
		m_pTexture( pTexture ),
		Model( pDevice, pDeviceContext, pShader )
	{}

	void TexturedModel::Draw() const
	{
		m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_VertexBuffer.Bind( m_pDeviceContext );
		m_IndexBuffer.Bind( m_pDeviceContext );

		auto vp = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();
		TextureShaderParameters params( DirectX::XMMatrixTranspose( GetWorldMatrix() ), DirectX::XMMatrixTranspose( vp ), m_pTexture, m_IndexBuffer.GetIndexCount() );
		m_pShader->Render( m_pDeviceContext, &params );
	}
}