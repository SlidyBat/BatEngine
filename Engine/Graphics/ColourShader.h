/*#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

#include "VertexShader.h"
#include "PixelShader.h"

namespace Bat
{
	class ColourShader
	{
	public:
		ColourShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename );
		ColourShader( const ColourShader& src ) = delete;
		ColourShader& operator=( const ColourShader& src ) = delete;
		ColourShader( ColourShader&& donor ) = delete;
		ColourShader& operator=( ColourShader&& donor ) = delete;

		bool Render( ID3D11DeviceContext* pDeviceContext, size_t nVertices, const DirectX::XMMATRIX& mat );
		bool RenderIndexed( ID3D11DeviceContext* pDeviceContext, size_t nIndexes, const DirectX::XMMATRIX& mat );
	private:
		VertexShader m_VertexShader;
		PixelShader m_PixelShader;
	};
}*/