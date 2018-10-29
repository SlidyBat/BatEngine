#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

#include "VertexShader.h"
#include "PixelShader.h"

class TextureShader
{
public:
	TextureShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename );
	TextureShader( const TextureShader& src ) = delete;
	TextureShader& operator=( const TextureShader& src ) = delete;
	TextureShader( TextureShader&& donor ) = delete;
	TextureShader& operator=( TextureShader&& donor ) = delete;

	bool Render( ID3D11DeviceContext* pDeviceContext, size_t nVertices, ID3D11ShaderResourceView* pTexture );
	bool RenderIndexed( ID3D11DeviceContext* pDeviceContext, size_t nIndexes, ID3D11ShaderResourceView* pTexture );
private:
	VertexShader m_VertexShader;
	PixelShader m_PixelShader;
};