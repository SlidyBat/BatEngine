#include "Model.h"

#include "TextureShader.h"

TexturedModel::TexturedModel( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IShader* pShader, Texture* pTexture, const std::vector<TexVertex>& verts, const std::vector<int>& indices )
	:
	m_VertexBuffer(pDevice, verts),
	m_IndexBuffer(pDevice, indices),
	m_pTexture(pTexture),
	Model( pDevice, pDeviceContext, pShader )
{}

void TexturedModel::Draw( const DirectX::XMMATRIX& vp ) const
{
	m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_VertexBuffer.Bind( m_pDeviceContext );
	m_IndexBuffer.Bind( m_pDeviceContext );
	TextureShaderParameters params( DirectX::XMMatrixTranspose( GetWorldMatrix() * vp ), m_pTexture, m_IndexBuffer.GetIndexCount() );
	m_pShader->Render( m_pDeviceContext, &params );
}
