#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

class ColourShader
{
private:
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};
public:
	ColourShader() = default;
	~ColourShader();
	ColourShader( const ColourShader& src ) = delete;
	ColourShader& operator=( const ColourShader& src ) = delete;
	ColourShader( ColourShader&& donor ) = delete;
	ColourShader& operator=( ColourShader&& donor ) = delete;

	bool Initialize( ID3D11Device* pDevice, HWND hWnd );
	bool Render( ID3D11DeviceContext* pDeviceContext, int nIndexes, DirectX::XMMATRIX worldMat, DirectX::XMMATRIX viewMat, DirectX::XMMATRIX projectionMat );
private:
	bool InitializeShader( ID3D11Device* pDevice, HWND hWnd, const std::wstring& vsFilename, const std::wstring& psFilename );
	void OutputShaderErrorMessage( ID3D10Blob* errorMessage, HWND hWnd, const std::wstring& shaderFilename );

	bool SetShaderParameters( ID3D11DeviceContext* pDeviceContext, DirectX::XMMATRIX worldMat, DirectX::XMMATRIX viewMat, DirectX::XMMATRIX projectionMat );
	void RenderShader( ID3D11DeviceContext* pDeviceContext, int nIndexes );
private:
	ID3D11VertexShader*	m_pVertexShader = nullptr;
	ID3D11PixelShader*	m_pPixelShader = nullptr;
	ID3D11InputLayout*	m_pInputLayout = nullptr;
	ID3D11Buffer*		m_pMatrixBuffer = nullptr;
};