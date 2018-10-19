#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>

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
	void OutputShaderErrorMessage( ID3DBlob* errorMessage, HWND hWnd, const std::wstring& shaderFilename );
	void SetShaderParameters( ID3D11DeviceContext* pDeviceContext, ID3D11ShaderResourceView* pSrv );

	void RenderShader( ID3D11DeviceContext* pDeviceContext, size_t nVertices );
	void RenderShaderIndexed( ID3D11DeviceContext* pDeviceContext, size_t nIndexes );
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_pInputLayout;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_pSamplerState;
};