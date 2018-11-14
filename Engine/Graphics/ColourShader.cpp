#include "ColourShader.h"
#include "Vertex.h"
#include <fstream>
#include "COMException.h"

namespace Bat
{
	ColourShader::ColourShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename )
		:
		m_VertexShader( pDevice, vsFilename, Vertex::InputLayout, Vertex::Inputs ),
		m_PixelShader( pDevice, psFilename )
	{}

	bool ColourShader::Render( ID3D11DeviceContext* pDeviceContext, size_t nVertices, const DirectX::XMMATRIX& mat )
	{
		m_VertexShader.GetConstantBuffer( 0 ).SetData( pDeviceContext, &mat );

		m_VertexShader.Bind( pDeviceContext );
		m_PixelShader.Bind( pDeviceContext );
		pDeviceContext->Draw( (UINT)nVertices, 0 );

		return true;
	}

	bool ColourShader::RenderIndexed( ID3D11DeviceContext* pDeviceContext, size_t nIndexes, const DirectX::XMMATRIX& mat )
	{
		m_VertexShader.GetConstantBuffer( 0 ).SetData( pDeviceContext, &mat );

		m_VertexShader.Bind( pDeviceContext );
		m_PixelShader.Bind( pDeviceContext );
		pDeviceContext->DrawIndexed( (UINT)nIndexes, 0, 0 );

		return true;
	}
}