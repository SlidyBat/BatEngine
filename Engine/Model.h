#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Texture.h"
#include "TexVertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "IShader.h"

class Model
{
public:
	Model( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IShader* pShader )
		:
		m_pDevice(pDevice),
		m_pDeviceContext(pDeviceContext),
		m_pShader(pShader)
	{
		UpdateWorldMatrix();
	}
	virtual ~Model() = default;

	virtual void Draw( const DirectX::XMMATRIX& vp ) const = 0;

	DirectX::XMFLOAT3 GetPosition() const
	{
		return m_vecPosition;
	}
	void SetPosition( const float x, const float y, const float z )
	{
		m_vecPosition = { x, y, z };
		UpdateWorldMatrix();
	}
	void SetPosition( const DirectX::XMFLOAT3& pos )
	{
		m_vecPosition = pos;
		UpdateWorldMatrix();
	}
	void MoveBy( const float dx, const float dy, const float dz )
	{
		m_vecPosition.x += dx;
		m_vecPosition.y += dy;
		m_vecPosition.z += dz;
		UpdateWorldMatrix();
	}

	DirectX::XMFLOAT3 GetRotation() const
	{
		return m_angRotation;
	}
	void SetRotation( const float pitch, const float yaw, const float roll )
	{
		m_angRotation = { pitch, yaw, roll };
		UpdateWorldMatrix();
	}
	void SetRotation( const DirectX::XMFLOAT3& ang )
	{
		m_angRotation = ang;
		UpdateWorldMatrix();
	}
	void RotateBy( const float dpitch, const float dyaw, const float droll )
	{
		m_angRotation.x += dpitch;
		m_angRotation.y += dyaw;
		m_angRotation.z += droll;
		UpdateWorldMatrix();
	}
protected:
	DirectX::XMMATRIX GetWorldMatrix() const
	{
		return m_matWorldMatrix;
	}
	void UpdateWorldMatrix()
	{
		m_matWorldMatrix = DirectX::XMMatrixRotationRollPitchYaw( m_angRotation.x, m_angRotation.y, m_angRotation.z ) *
			DirectX::XMMatrixTranslation( m_vecPosition.x, m_vecPosition.y, m_vecPosition.z );
	}
protected:
	DirectX::XMFLOAT3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_angRotation = { 0.0f, 0.0f, 0.0f };
	DirectX::XMMATRIX m_matWorldMatrix;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IShader* m_pShader;
};

class TexturedModel : public Model
{
public:
	TexturedModel( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IShader* pShader, Texture* pTexture, const std::vector<TexVertex>& verts, const std::vector<int>& indices );

	virtual void Draw( const DirectX::XMMATRIX& vp ) const override;
private:
	VertexBuffer<TexVertex> m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	Texture* m_pTexture;
};

/*template <typename V>
class Model
{
public:
	Model( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::vector<V>& verts, const std::vector<int>& indices, D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST )
		:
		m_Vertices( verts ),
		m_Indices( indices ),
		m_PrimitiveTopology( topology ),
		m_IndexBuffer( pDevice, indices ),
		m_VertexBuffer( pDevice, verts )
	{}

	Model( const Model& src ) = delete;
	Model& operator=( const Model& src ) = delete;
	Model( Model&& donor ) = delete;
	Model& operator=( Model&& donor ) = delete;

	void Draw()
	{
		m_VertexBuffer.Bind( m_pDeviceContext );
		m_IndexBuffer.Bind( m_pDeviceContext );
		m_pDeviceContext->IASetPrimitiveTopology( m_PrimitiveTopology );

		m_pDeviceContext->DrawIndexed( GetIndexCount(), GetVertexCount(), 0 );
	}

	size_t GetIndexCount() const
	{
		return m_Indices.size();
	}
	size_t GetVertexCount() const
	{
		return m_Vertices.size();
	}
	D3D_PRIMITIVE_TOPOLOGY GetTopologyType() const
	{
		return m_PrimitiveTopology;
	}
private:
private:
	VertexBuffer<V> m_VertexBuffer;
	IndexBuffer m_IndexBuffer;

	D3D_PRIMITIVE_TOPOLOGY					m_PrimitiveTopology;
	std::vector<V>							m_Vertices;
	std::vector<int>						m_Indices;

	ID3D11Device*							m_pDevice;
	ID3D11DeviceContext*					m_pDeviceContext;
};*/