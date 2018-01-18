#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>

class ColourShader
{
public:
	ColourShader() = default;
	~ColourShader();
	ColourShader( const ColourShader& src ) = delete;
	ColourShader& operator=( const ColourShader& src ) = delete;
	ColourShader( ColourShader&& donor ) = delete;
	ColourShader& operator=( ColourShader&& donor ) = delete;

	bool Initialize( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename );
	bool Render( ID3D11DeviceContext* pDeviceContext, int nVertices );
private:
	void OutputShaderErrorMessage( ID3DBlob* errorMessage, HWND hWnd, const std::wstring& shaderFilename );
	void RenderShader( ID3D11DeviceContext* pDeviceContext, int nVertices );
private:
	ID3D11VertexShader*	m_pVertexShader = nullptr;
	ID3D11PixelShader*	m_pPixelShader = nullptr;
	ID3D11InputLayout*	m_pInputLayout = nullptr;
};