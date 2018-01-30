#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

class ColourShader
{
public:
	ColourShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename );
	ColourShader( const ColourShader& src ) = delete;
	ColourShader& operator=( const ColourShader& src ) = delete;
	ColourShader( ColourShader&& donor ) = delete;
	ColourShader& operator=( ColourShader&& donor ) = delete;

	bool Render( ID3D11DeviceContext* pDeviceContext, UINT nVertices );
	bool RenderIndexed( ID3D11DeviceContext* pDeviceContext, UINT nIndexes );
private:
	void OutputShaderErrorMessage( ID3DBlob* errorMessage, HWND hWnd, const std::wstring& shaderFilename );
	void RenderShader( ID3D11DeviceContext* pDeviceContext, UINT nVertices );
	void RenderShaderIndexed( ID3D11DeviceContext* pDeviceContext, UINT nIndexes );
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_pInputLayout;
};