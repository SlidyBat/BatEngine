#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

class Model
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 colour;
	};
public:
	Model() = default;
	~Model();
	Model( const Model& src ) = delete;
	Model& operator=( const Model& src ) = delete;
	Model( Model&& donor ) = delete;
	Model& operator=( Model&& donor ) = delete;

	bool Initialize( ID3D11Device* pDevice );
	void Render( ID3D11DeviceContext* pDeviceContext );

	int GetIndexCount() const;
private:
	bool InitializeBuffers( ID3D11Device* pDevice );
	void RenderBuffers( ID3D11DeviceContext* pDeviceContext );
private:
	ID3D11Buffer*	m_pVertexBuffer = nullptr;
	ID3D11Buffer*	m_pIndexBuffer = nullptr;

	int m_nVertexCount;
	int m_nIndexCount;
};