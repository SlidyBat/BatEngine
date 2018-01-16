#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

class ColourShader
{
private:
public:
	ColourShader() = default;
	~ColourShader();
	ColourShader( const ColourShader& src ) = delete;
	ColourShader& operator=( const ColourShader& src ) = delete;
	ColourShader( ColourShader&& donor ) = delete;
	ColourShader& operator=( ColourShader&& donor ) = delete;

	bool Initialize( ID3D11Device* pDevice, HWND hWnd );
	bool Render( ID3D11DeviceContext* pDeviceContext, int nIndexes );
private:
	bool InitializeShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename );
	void OutputShaderErrorMessage( ID3D10Blob* errorMessage, HWND hWnd, const std::wstring& shaderFilename );

	void RenderShader( ID3D11DeviceContext* pDeviceContext, int nnVertices );
private:
	ID3D11VertexShader*	m_pVertexShader = nullptr;
	ID3D11PixelShader*	m_pPixelShader = nullptr;
	ID3D11InputLayout*	m_pInputLayout = nullptr;
};